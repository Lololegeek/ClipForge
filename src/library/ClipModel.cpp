#include "ClipModel.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
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
            {Game, "game"},           {Favorite, "favorite"}, {Tags, "tags"},     {Missing, "missing"}};
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
        return !QFileInfo::exists(v["path"].toString());
    default:
        return {};
    }
}
QString ClipModel::where() const {
    return " WHERE (name LIKE :search OR game LIKE :search OR tags LIKE :search)" +
           QString(favorites_ ? " AND favorite=1" : "");
}
void ClipModel::filter(QString s, QString o, bool f) {
    search_ = s;
    order_ = QStringList{"created", "duration", "size", "name"}.contains(o) ? o : "created";
    favorites_ = f;
    reload();
}
void ClipModel::reload() {
    beginResetModel();
    rows_.clear();
    QSqlQuery q(db_);
    q.prepare("SELECT COUNT(*) FROM clips" + where());
    q.bindValue(":search", "%" + search_ + "%");
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
    q.bindValue(":search", "%" + search_ + "%");
    q.bindValue(":offset", rows_.size());
    if (!q.exec())
        return;
    QVector<QVariantMap> page;
    while (q.next()) {
        QVariantMap m;
        auto r = q.record();
        for (int i = 0; i < r.count(); ++i)
            m[r.fieldName(i)] = q.value(i);
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
    q.prepare("INSERT INTO clips(path,name,duration,size,created,game) "
              "VALUES(:path,:name,:duration,:size,:created,:game) ON CONFLICT(path) DO UPDATE SET "
              "duration=excluded.duration,size=excluded.size");
    for (auto key : QStringList{"path", "name", "duration", "size", "created", "game"})
        q.bindValue(":" + key, m.value(key));
    if (!q.exec())
        emit failure(q.lastError().text());
    reload();
}
void ClipModel::setThumbnail(QString p, QString t) {
    QSqlQuery q(db_);
    q.prepare("UPDATE clips SET thumbnail=? WHERE path=?");
    q.addBindValue(t);
    q.addBindValue(p);
    q.exec();
    reload();
}
bool ClipModel::favorite(int id, bool v) {
    QSqlQuery q(db_);
    q.prepare("UPDATE clips SET favorite=? WHERE id=?");
    q.addBindValue(v);
    q.addBindValue(id);
    bool ok = q.exec();
    reload();
    return ok;
}
bool ClipModel::tags(int id, QString t) {
    QSqlQuery q(db_);
    q.prepare("UPDATE clips SET tags=? WHERE id=?");
    q.addBindValue(t.left(512));
    q.addBindValue(id);
    bool ok = q.exec();
    reload();
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
    reload();
    return ok;
}
} // namespace cf
