#include "logtreemodel.h"

LogTreeModel::LogTreeModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    diff = 0;
    rows_cnt = 0;
    column_cnt = 4;
    max_rows = 1000;
}

LogTreeModel::~LogTreeModel()
{
}

QVariant LogTreeModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    case Qt::DisplayRole:
    {
        int id = root_nodes.value(index,-1);
        if(id < 0) return QVariant();
        return root_values.value(id);
    }

    default: return QVariant();
    }
}

int LogTreeModel::rowCount(const QModelIndex &parent) const
{
    if(parent == QModelIndex())
        return rows_cnt;
    int id = root_nodes.value(parent,-1);
    return links.keys(id-diff).size();
}

int LogTreeModel::columnCount(const QModelIndex &parent) const
{
    if(parent == QModelIndex())
        return column_cnt;

    return 1;
}

QModelIndex LogTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent == QModelIndex())
    {
        QModelIndex idx = root_nodes.key(column+(row*columnCount()),QModelIndex());
        qDebug()<<idx<<row<<column<<"control"<<column+(row*columnCount());
        return idx;
    }

    int id = root_nodes.value(parent,-1);
    if(id > -1)
    {
        id += diff;
        QModelIndexList idxs = links.keys(id);
        QModelIndex idx;
        foreach(idx,idxs)
            if(idx.row() == row && idx.column() == column)
                return idx;
    }
    return QModelIndex();
}

QModelIndex LogTreeModel::parent(const QModelIndex &child) const
{
    int prnt_id = links.value(child,-1);
    if(prnt_id == -1) return QModelIndex();

    prnt_id += diff;
    QModelIndex prnt = root_nodes.key(prnt_id, QModelIndex());
    return prnt;
}

bool LogTreeModel::hasChildren(const QModelIndex &parent) const
{
    if(parent == QModelIndex() && rowCount()) return true;
    int prnt_id = root_nodes.value(parent, -1);
    if(prnt_id < 0) return false;

    prnt_id +=diff;
    QModelIndex idx = links.key(prnt_id,QModelIndex());
    if(idx != QModelIndex())
        return true;

    return false;
}

bool LogTreeModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    if(LogTreeModel::index(row,column,parent) == QModelIndex())
        return false;

    return true;
}

Qt::ItemFlags LogTreeModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index);
}

bool LogTreeModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if(row == rowCount(parent))
    {
        beginInsertRows(parent,row,row+count);

        for(int i = 0; i < count; ++i)
        {
            for(int y = 0; y < columnCount(parent); ++y)
            {
                int new_id = root_nodes.size() + sub_nodes.size();
                QModelIndex newindex = LogTreeModel::createIndex(row+i,y, new_id);
                root_nodes.insert(newindex,row+i*columnCount(parent)+y);
                root_values.append(QVariant(1));
            }
            ++rows_cnt;
        }
        endInsertRows();
        //qDebug()<<root_nodes;
        return true;
    }
    return false;
}

bool LogTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    return true;
}

Qt::DropActions LogTreeModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions();
}

bool LogTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    return QAbstractItemModel::dropMimeData(data,action,row,column,parent);
}

QMimeData * LogTreeModel::mimeData(const QModelIndexList &indexes) const
{
    return QAbstractItemModel::mimeData(indexes);
}

QStringList LogTreeModel::mimeTypes() const
{
    return QAbstractItemModel::mimeTypes();
}

bool LogTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    return false;
}

void LogTreeModel::clearLog()
{
    rows_cnt = 0;
    root_nodes.clear();
    root_values.clear();
    diff = 0;
    sub_nodes.clear();
    links.clear();
}

void LogTreeModel::appendLog(int id_task, int ms_type, const QString &title, const QString &more)
{
}

