#ifndef LOGTREEMODEL_H
#define LOGTREEMODEL_H

#include <QAbstractItemModel>
#include <QDebug>
#include <QStringList>

class LogTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit LogTreeModel(QObject *parent = 0);
    virtual ~LogTreeModel();

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual bool hasChildren(const QModelIndex &parent) const;
    virtual bool hasIndex(int row, int column, const QModelIndex &parent) const;
    virtual Qt::ItemFlags flags(const QModelIndex & index) const;
    virtual bool insertRows(int row, int count, const QModelIndex & parent = QModelIndex());
    virtual bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::DropActions supportedDropActions() const;
    virtual bool dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent);
    virtual QMimeData* mimeData(const QModelIndexList & indexes)const;
    QStringList mimeTypes()const;
    virtual bool removeRows(int row, int count, const QModelIndex & parent = QModelIndex());

signals:

public slots:
    void clearLog();
    void appendLog(int id_task, int ms_type, const QString &title, const QString &more);

private:
    int max_rows;
    int rows_cnt;
    int column_cnt;
    int diff;

    QHash<QModelIndex, int> root_nodes; //основные узлы дерева
    QList<QVariant> root_values; //значения узлов
    QHash<QModelIndex, QVariant> sub_nodes; //дочерние узлы ветви дерева
    QHash<QModelIndex, int> links; //связи между дочерними и основными узлами (int - номер родительской строки)
};

#endif // LOGTREEMODEL_H
