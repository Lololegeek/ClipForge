#pragma once
#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QVariantMap>
namespace cf {
class ClipModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
  public:
    enum Role {
        Id = Qt::UserRole + 1,
        Name,
        Path,
        Url,
        Thumbnail,
        Duration,
        Size,
        Created,
        Game,
        Favorite,
        Tags,
        Missing
    };
    explicit ClipModel(QString database, QObject *p = nullptr);
    ~ClipModel();
    int rowCount(const QModelIndex &p = {}) const override;
    QVariant data(const QModelIndex &, int) const override;
    QHash<int, QByteArray> roleNames() const override;
    bool canFetchMore(const QModelIndex &p) const override;
    void fetchMore(const QModelIndex &p) override;
    int count() const;
    bool isOpen() const;
    QString error() const;
    Q_INVOKABLE void filter(QString search, QString order = "created", bool favorites = false);
    Q_INVOKABLE QVariantMap get(int index) const;
    Q_INVOKABLE bool favorite(int id, bool value);
    Q_INVOKABLE bool rename(int id, QString name);
    Q_INVOKABLE bool tags(int id, QString tags);
    Q_INVOKABLE bool remove(int id, bool trash = true);
    void upsert(const QVariantMap &);
    void setThumbnail(QString path, QString image);
    void reload();
    QHash<QString, qint64> knownFiles() const;
  signals:
    void countChanged();
    void failure(QString message);

  private:
    QSqlDatabase db_;
    QVector<QVariantMap> rows_;
    QString search_, order_ = "created";
    bool favorites_ = false;
    int total_ = 0;
    QString where() const;
    QString pattern() const;
    int rowOf(int id) const;
    void patch(int row, const QVariantMap &values, const QList<int> &roles);
};
} // namespace cf
