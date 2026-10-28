#include "ClipModel.h"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QUrl>
namespace cf {
ClipModel::ClipModel(QString path, QObject *p) : QAbstractListModel(p) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    db_ = QSqlDatabase::addDatabase("QSQLITE", "clips");
    db_.setDatabaseName(path);
    if (!db_.open())
        return;
    QSqlQuery q(db_);
    q.exec("PRAGMA journal_mode=WAL");
    q.exec("PRAGMA busy_timeout=3000");
    q.exec("CREATE TABLE IF NOT EXISTS clips(id INTEGER PRIMARY KEY,path TEXT UNIQUE NOT NULL,name "
           "TEXT,thumbnail TEXT,duration REAL,size INTEGER,created TEXT,game TEXT DEFAULT '',favorite "
           "INTEGER DEFAULT 0,tags TEXT DEFAULT '')");
    q.exec("CREATE INDEX IF NOT EXISTS clips_created ON clips(created DESC)");
    // 0.8: marker positions (seconds from the clip start, JSON array). Fails harmlessly once the column exists.
    q.exec("ALTER TABLE clips ADD COLUMN markers TEXT DEFAULT ''");
    reload();
}
ClipModel::~ClipModel() {
    db_.close();
    db_ = QSqlDatabase();
    QSqlDatabase::removeDatabase("clips");
}
bool ClipModel::isOpen() const {
    return db_.isOpen();
}
QString ClipModel::error() const {
    return db_.lastError().text();
}
int ClipModel::rowCount(const QModelIndex &p) const {
    return p.isValid() ? 0 : int(rows_.size());
}
int ClipModel::count() const {
    return total_;
}
QHash<int, QByteArray> ClipModel::roleNames() const {
    return {{Id, "clipId"},           {Name, "clipName"},     {Path, "filePath"}, {Url, "fileUrl"},
            {Thumbnail, "thumbnail"}, {Duration, "duration"}, {Size, "fileSize"}, {Created, "created"},
            {Game, "game"},           {Favorite, "favorite"}, {Tags, "tags"},     {Missing, "missing"},
            {Markers, "markers"}};
}
QVariant ClipModel::data(const QModelIndex &i, int r) const {
    if (i.row() < 0 || i.row() >= rows_.size())
        return {};
    const auto &v = rows_[i.row()];
    switch (r) {
    case Id:
        return v["id"];
    case Name:
        return v["name"];
    case Path:
        return v["path"];
    case Url:
        return QUrl::fromLocalFile(v["path"].toString());
    case Thumbnail:
        return QUrl::fromLocalFile(v["thumbnail"].toString());
    case Duration:
        return v["duration"];
    case Size:
        return v["size"];
    case Created:
        return v["created"];
    case Game:
        return v["game"];
    case Favorite:
        return v["favorite"].toBool();
    case Tags:
        return v["tags"];
    case Missing:
        return v["missing"];
    case Markers:
        return QJsonDocument::fromJson(v["markers"].toString().toUtf8()).array().toVariantList();
    default:
        return {};
    }
}
QString ClipModel::where() const {
    return " WHERE (name LIKE :search ESCAPE '!' OR game LIKE :search ESCAPE '!' OR tags LIKE :search ESCAPE '!')" +
           QString(favorites_ ? " AND favorite=1" : "") + QString(game_.isNull() ? "" : " AND game=:game");
}
QString ClipModel::pattern() const {
    // Typed % and _ are literal characters, not SQL wildcards.
    auto s = search_;
    s.replace("!", "!!").replace("%", "!%").replace("_", "!_");
    return "%" + s + "%";
}
int ClipModel::rowOf(int id) const {
    for (int i = 0; i < rows_.size(); ++i)
        if (rows_[i]["id"].toInt() == id)
            return i;
    return -1;
}
void ClipModel::patch(int row, const QVariantMap &values, const QList<int> &roles) {
    if (row < 0 || row >= rows_.size())
        return;
    for (auto it = values.begin(); it != values.end(); ++it)
        rows_[row][it.key()] = it.value();
    emit dataChanged(index(row), index(row), roles);
}
QHash<QString, qint64> ClipModel::knownFiles() const {
    QHash<QString, qint64> known;
    QSqlQuery q(db_);
    if (q.exec("SELECT path,size FROM clips WHERE thumbnail IS NOT NULL AND thumbnail<>''"))
        while (q.next())
            known.insert(q.value(0).toString(), q.value(1).toLongLong());
    return known;
}
void ClipModel::filter(QString s, QString o, bool f) {
    search_ = s;
    order_ = QStringList{"created", "duration", "size", "name"}.contains(o) ? o : "created";
    favorites_ = f;
    reload();
}
void ClipModel::filterGame(QString game) {
    // A null string shows every game; an empty one the clips without a detected game.
    game_ = game == "*" ? QString() : game.isNull() ? QString("") : game;
    reload();
}
QVariantList ClipModel::gameList() const {
    QVariantList games;
    QSqlQuery q(db_);
    if (q.exec("SELECT game,COUNT(*) FROM clips GROUP BY game ORDER BY COUNT(*) DESC, game ASC"))
        while (q.next())
            games << QVariantMap{{"game", q.value(0).toString()}, {"count", q.value(1).toInt()}};
    return games;
}
void ClipModel::reload() {
    beginResetModel();
    rows_.clear();
    QSqlQuery q(db_);
    q.prepare("SELECT COUNT(*) FROM clips" + where());
    q.bindValue(":search", pattern());
    if (!game_.isNull())
        q.bindValue(":game", game_);
    total_ = q.exec() && q.next() ? q.value(0).toInt() : 0;
    endResetModel();
    fetchMore({});
    emit countChanged();
}
bool ClipModel::canFetchMore(const QModelIndex &p) const {
    return !p.isValid() && rows_.size() < total_;
}
void ClipModel::fetchMore(const QModelIndex &p) {
    if (p.isValid())
        return;
    QSqlQuery q(db_);
    q.prepare("SELECT * FROM clips" + where() + " ORDER BY " + order_ +
              (order_ == "name" ? " ASC" : " DESC") + " LIMIT 100 OFFSET :offset");
    q.bindValue(":search", pattern());
    if (!game_.isNull())
        q.bindValue(":game", game_);
    q.bindValue(":offset", rows_.size());
    if (!q.exec())
        return;
    QVector<QVariantMap> page;
    while (q.next()) {
        QVariantMap m;
        auto r = q.record();
        for (int i = 0; i < r.count(); ++i)
            m[r.fieldName(i)] = q.value(i);
        // Cached once per fetch: data() runs on every repaint and must not touch the disk.
        m["missing"] = !QFileInfo::exists(m["path"].toString());
        page << m;
    }
    if (page.isEmpty())
        return;
    int start = int(rows_.size());
    beginInsertRows({}, start, start + int(page.size()) - 1);
    rows_ += page;
    endInsertRows();
}
QVariantMap ClipModel::get(int i) const {
    return i >= 0 && i < rows_.size() ? rows_[i] : QVariantMap{};
}
void ClipModel::upsert(const QVariantMap &m) {
    QSqlQuery q(db_);
    q.prepare("INSERT INTO clips(path,name,duration,size,created,game,markers) "
              "VALUES(:path,:name,:duration,:size,:created,:game,:markers) ON CONFLICT(path) DO UPDATE SET "
              "duration=excluded.duration,size=excluded.size,"
              "markers=CASE WHEN excluded.markers<>'' THEN excluded.markers ELSE markers END");
    for (auto key : QStringList{"path", "name", "duration", "size", "created", "game"})
        q.bindValue(":" + key, m.value(key));
    q.bindValue(":markers", m.value("markers").toString());
    if (!q.exec()) {
        emit failure(q.lastError().text());
        return;
    }
    for (int i = 0; i < rows_.size(); ++i)
        if (rows_[i]["path"] == m.value("path")) {
            QVariantMap values{{"duration", m.value("duration")}, {"size", m.value("size")}, {"missing", false}};
            if (!m.value("markers").toString().isEmpty())
                values["markers"] = m.value("markers");
            patch(i, values, {Duration, Size, Missing, Markers});
            return;
        }
    reload();
}
void ClipModel::setThumbnail(QString p, QString t) {
    QSqlQuery q(db_);
    q.prepare("UPDATE clips SET thumbnail=? WHERE path=?");
    q.addBindValue(t);
    q.addBindValue(p);
    if (!q.exec())
        return;
    for (int i = 0; i < rows_.size(); ++i)
        if (rows_[i]["path"] == p)
            patch(i, {{"thumbnail", t}}, {Thumbnail});
}
bool ClipModel::favorite(int id, bool v) {
    QSqlQuery q(db_);
    q.prepare("UPDATE clips SET favorite=? WHERE id=?");
    q.addBindValue(v);
    q.addBindValue(id);
    bool ok = q.exec();
    if (ok && favorites_ && !v)
        reload();
    else if (ok)
        patch(rowOf(id), {{"favorite", v}}, {Favorite});
    return ok;
}
bool ClipModel::tags(int id, QString t) {
    QSqlQuery q(db_);
    q.prepare("UPDATE clips SET tags=? WHERE id=?");
    q.addBindValue(t.left(512));
    q.addBindValue(id);
    bool ok = q.exec();
    if (ok)
        patch(rowOf(id), {{"tags", t.left(512)}}, {Tags});
    return ok;
}
bool ClipModel::rename(int id, QString name) {
    if (name.trimmed().isEmpty() || name.contains(QRegularExpression("[<>:\"/\\\\|?*]")))
        return false;
    QSqlQuery q(db_);
    q.prepare("SELECT path FROM clips WHERE id=?");
    q.addBindValue(id);
    if (!q.exec() || !q.next())
        return false;
    auto old = q.value(0).toString();
    QFileInfo fi(old);
    auto path = fi.absolutePath() + "/" + name.trimmed() + "." + fi.suffix();
    if (!QFile::rename(old, path)) {
        emit failure("Renommage impossible : fichier utilisé ou nom déjà présent.");
        return false;
    }
    q.prepare("UPDATE clips SET path=?,name=? WHERE id=?");
    q.addBindValue(path);
    q.addBindValue(name.trimmed());
    q.addBindValue(id);
    bool ok = q.exec();
    if (ok && order_ != "name")
        patch(rowOf(id), {{"path", path}, {"name", name.trimmed()}, {"missing", false}}, {Path, Url, Name, Missing});
    else
        reload();
    return ok;
}
bool ClipModel::remove(int id, bool trash) {
    QSqlQuery q(db_);
    q.prepare("SELECT path FROM clips WHERE id=?");
    q.addBindValue(id);
    if (!q.exec() || !q.next())
        return false;
    auto p = q.value(0).toString();
    if (QFileInfo::exists(p) && !(trash ? QFile::moveToTrash(p) : QFile::remove(p))) {
        emit failure("Impossible de supprimer le fichier. Fermez son lecteur.");
        return false;
    }
    q.prepare("DELETE FROM clips WHERE id=?");
    q.addBindValue(id);
    bool ok = q.exec();
    int row = rowOf(id);
    if (ok && row >= 0) {
        beginRemoveRows({}, row, row);
        rows_.remove(row);
        endRemoveRows();
        --total_;
        emit countChanged();
    } else if (ok)
        reload();
    return ok;
}
int ClipModel::favoriteMany(QVariantList ids, bool value) {
    int done = 0;
    for (const auto &id : ids)
        done += QSqlQuery(db_).exec(QString("UPDATE clips SET favorite=%1 WHERE id=%2").arg(value ? 1 : 0).arg(id.toInt()));
    reload();
    return done;
}
int ClipModel::tagMany(QVariantList ids, QString tag) {
    tag = tag.trimmed();
    if (tag.isEmpty())
        return 0;
    int done = 0;
    for (const auto &id : ids) {
        QSqlQuery q(db_);
        q.prepare("SELECT tags FROM clips WHERE id=?");
        q.addBindValue(id.toInt());
        if (!q.exec() || !q.next())
            continue;
        // Tags are comma separated; adding one keeps the existing ones and never duplicates it.
        QStringList tags;
        for (const auto &t : q.value(0).toString().split(',', Qt::SkipEmptyParts))
            tags << t.trimmed();
        if (!tags.contains(tag, Qt::CaseInsensitive))
            tags << tag;
        done += this->tags(id.toInt(), tags.join(", "));
    }
    return done;
}
int ClipModel::removeMany(QVariantList ids) {
    int done = 0;
    for (const auto &id : ids)
        done += remove(id.toInt());
    return done;
}
int ClipModel::cleanup(qint64 maxBytes, int maxDays) {
    if (maxBytes <= 0 && maxDays <= 0)
        return 0;
    struct Entry {
        int id;
        qint64 size;
        QString created;
        bool favorite;
    };
    QVector<Entry> all;
    qint64 total = 0;
    QSqlQuery q(db_);
    if (!q.exec("SELECT id,size,created,favorite FROM clips ORDER BY created ASC"))
        return 0;
    while (q.next()) {
        all.push_back({q.value(0).toInt(), q.value(1).toLongLong(), q.value(2).toString(), q.value(3).toBool()});
        total += all.back().size;
    }
    const QString limit =
        maxDays > 0 ? QDateTime::currentDateTime().addDays(-maxDays).toString(Qt::ISODateWithMs) : QString();
    int removed = 0;
    for (const auto &e : all) {
        if (e.favorite)
            continue;
        const bool tooOld = !limit.isEmpty() && e.created < limit;
        const bool overQuota = maxBytes > 0 && total > maxBytes;
        if (!tooOld && !overQuota)
            continue;
        if (remove(e.id)) {
            total -= e.size;
            ++removed;
        }
    }
    return removed;
}
} // namespace cf
