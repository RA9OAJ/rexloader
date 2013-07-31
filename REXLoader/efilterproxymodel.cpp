#include "efilterproxymodel.h"

EFilterProxyModel::EFilterProxyModel(QObject *parent) :
    QAbstractProxyModel(parent)
{
    setParent(parent);
}

QModelIndex EFilterProxyModel::buddy(const QModelIndex &index) const
{
}

bool EFilterProxyModel::canFetchMore(const QModelIndex &parent) const
{

}

int EFilterProxyModel::columnCount(const QModelIndex &parent) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
            return sourceModel()->columnCount(parent);
    }
    //-----Доработать---------
}

QVariant EFilterProxyModel::data(const QModelIndex &index, int role) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
            return sourceModel()->data(index,role);
    }
    //-----Доработать---------
}

void EFilterProxyModel::fetchMore(const QModelIndex &parent)
{

}

Qt::ItemFlags EFilterProxyModel::flags(const QModelIndex &index) const
{
}

bool EFilterProxyModel::hasChildren(const QModelIndex &parent) const
{
}

QVariant EFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
}

QModelIndex EFilterProxyModel::index(int row, int column, const QModelIndex &parent) const
{
}

bool EFilterProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
}

bool EFilterProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
}

QModelIndex EFilterProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
}

QItemSelection EFilterProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
}

QItemSelection EFilterProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
}

QModelIndex EFilterProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
}

QModelIndexList EFilterProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
}

QMimeData *EFilterProxyModel::mimeData(const QModelIndexList &indexes) const
{
}

QStringList EFilterProxyModel::mimeTypes() const
{
}

QModelIndex EFilterProxyModel::parent(const QModelIndex &child) const
{
}

bool EFilterProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
}

bool EFilterProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
}

int EFilterProxyModel::rowCount(const QModelIndex &parent) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
            return sourceModel()->rowCount(parent);
    }
    //-----Доработать---------
}

bool EFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(sourceModel())
    {
        return sourceModel()->setData(mapToSource(index),value, role);
    }
    return false;
}

bool EFilterProxyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
}

void EFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    _src = sourceModel;
}

void EFilterProxyModel::sort(int column, Qt::SortOrder order)
{
}

QSize EFilterProxyModel::span(const QModelIndex &index) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
            return sourceModel()->span(index);
    }
    //-----Доработать---------
}

Qt::DropActions EFilterProxyModel::supportedDropActions() const
{
    if(sourceModel())
        return sourceModel()->supportedDragActions();
    return Qt::IgnoreAction;
}

QAbstractItemModel *EFilterProxyModel::sourceModel() const
{
    return _src;
}

void EFilterProxyModel::addFilter(int key_column, int filter_role, int _operator_, const QVariant &filter_val)
{
    if(!sourceModel())
        return;

    if(sourceModel()->columnCount() <= key_column)
        return;

    EFFilter cur;
    cur.data_role = filter_role;
    cur.filter_operator = _operator_;
    cur.filter_value = filter_val;

    QList<EFFilter> list = _filters.values(filter_role);
    if(list.isEmpty())
        return;

    foreach (EFFilter fltr, list) {
        if (fltr == cur)
            return;
    }
    _filters.insertMulti(key_column,cur);
    //Доработать реакцию на смену фильтров + сортировка
}

void EFilterProxyModel::clearFilter(int key_column)
{
    _filters.remove(key_column);
    //Доработать реакцию на смену фильтров + сортировка
}

void EFilterProxyModel::clearAllFilters()
{
    _filters.clear();
    //Доработать реакцию на смену фильтров + сортировка
}
