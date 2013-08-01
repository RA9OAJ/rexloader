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
    QModelIndex cindex;
    if(sourceModel() && _srcmap.contains(sourceIndex.row()))
    {
        int row = _srcmap.indexOf(sourceIndex.row());
        QModelIndex prnt;
        if(sourceIndex.parent() != QModelIndex())
            prnt = mapFromSource(sourceIndex.parent());

        cindex = index(row,sourceIndex.column(),prnt);
    }
    return cindex;
}

QItemSelection EFilterProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
}

QItemSelection EFilterProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
}

QModelIndex EFilterProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    QModelIndex eindex;
    if(sourceModel() && _srcmap.indexOf(proxyIndex.row()) != -1)
    {
        int row = _srcmap.value(proxyIndex.row());
        QModelIndex prnt;
        if(proxyIndex.parent() != QModelIndex())
            prnt = mapToSource(proxyIndex.parent());

        eindex = _src->index(row,proxyIndex.column(),prnt);
    }
    return eindex;
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
    QModelIndex prnt;
    if(sourceModel())
    {
        QModelIndex eidx = mapToSource(child);
        prnt = _src->parent(eidx);
        prnt = mapFromSource(prnt);
    }
    return prnt;
}

bool EFilterProxyModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    if(sourceModel())
    {
        QModelIndex eprnt = mapToSource(parent);
        beginRemoveColumns(parent,column,column + count - 1);
        sourceModel()->removeColumns(column,count,eprnt);
        //Тут проверям фильтры по колонкам и удаляем, если удалена колонка
        //по которой идет фильтрация или сортировка
        int fsz = _filters.size();
        for(int i = column; i < column + count; ++i)
        {
            if(_filters.contains(i))
                _filters.remove(i);

            if(_sort_param.first == i)
                _sort_param.first = -1;
        }

        if(fsz > _filters.size())
        {
            runFiltering();
            runSorting(_sort_param.first,(Qt::SortOrder)_sort_param.second);
        }
        else
        {

        }

        endRemoveColumns();
    }
    return false;
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
    _sort_param.first = column;
    _sort_param.second = order;
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

void EFilterProxyModel::runSorting(int column, Qt::SortOrder order)
{
}

void EFilterProxyModel::runFiltering()
{
}
