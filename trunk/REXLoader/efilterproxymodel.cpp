#include "efilterproxymodel.h"

EFilterProxyModel::EFilterProxyModel(QObject *parent) :
    QAbstractProxyModel(parent)
{
    setParent(parent);
}

QModelIndex EFilterProxyModel::buddy(const QModelIndex &index) const
{
    QModelIndex bdindex;
    if(sourceModel())
    {
        bdindex = sourceModel()->buddy(mapToSource(index));
        bdindex = mapFromSource(bdindex);
    }
    return bdindex;
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

    int cnt = 0;
    if(indexes.contains(parent))
        cnt = indexes.value(parent).value(0).size();

    return cnt;
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
    QModelIndexList lst = _srcmap.values();
    if(sourceModel())
    {
        QModelIndex canon = sourceIndex;
        if(sourceIndex.column() != 0)
            canon = sourceModel()->index(sourceIndex.row(),0,sourceIndex.parent());
        cindex = _srcmap.key(canon,QModelIndex());
        cindex = index(cindex.row(),sourceIndex.column(),cindex.parent());
    }
    return cindex;
}

QItemSelection EFilterProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
    return QItemSelection();
}

QItemSelection EFilterProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
    return QItemSelection();
}

QModelIndex EFilterProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    QModelIndex eindex;
    if(sourceModel() && !indexes.isEmpty())
    {
        QModelIndex canon = proxyIndex;
        if(proxyIndex.column() != 0)
            canon = index(proxyIndex.row(),0,proxyIndex.parent());
        eindex = _srcmap.value(canon,QModelIndex());
        eindex = sourceModel()->index(eindex.row(),proxyIndex.column(),eindex.parent());
    }
    return eindex;
}

QModelIndexList EFilterProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    return QModelIndexList();
}

QMimeData *EFilterProxyModel::mimeData(const QModelIndexList &indexes) const
{
    return 0;
}

QStringList EFilterProxyModel::mimeTypes() const
{
    return QStringList();
}

QModelIndex EFilterProxyModel::parent(const QModelIndex &child) const
{
    QModelIndex prnt;
    if(sourceModel())
    {
        QModelIndex eidx = mapToSource(child);
        prnt = sourceModel()->parent(eidx);
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
            //runSorting(_sort_param.first,(Qt::SortOrder)_sort_param.second); //?
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
    return false;
}

int EFilterProxyModel::rowCount(const QModelIndex &parent) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
            return sourceModel()->rowCount(parent);
    }
    int cnt = 0;
    if(indexes.contains(parent))
        cnt = indexes.value(parent).size();

    return cnt;
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
    return false;
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

bool EFilterProxyModel::runFiltering(int row, const QModelIndex &parent)
{
    int cnt = 0;
    if(sourceModel() && !_filters.isEmpty())
    {
        for(int i = row; row < sourceModel()->rowCount(parent); ++i)
        {
            QModelIndex nparent = sourceModel()->index(i,0,parent);
            if(sourceModel()->rowCount(nparent))
            {
                addRow(i,parent);
                if(!runFiltering(0,nparent) && !matchFilters(i,parent)) //если не прошли проверку подстроки и текущая строка
                    deleteRow(i,parent);
                else ++cnt;
            }
            else if(matchFilters(i,parent))
            {
                addRow(i,parent);
                ++cnt;
            }
        }
    }
    return cnt;
}

bool EFilterProxyModel::matchFilters(int row, const QModelIndex &parent) const
{
    if(sourceModel())
    {
    }
    return false;
}

void EFilterProxyModel::addRow(int row, const QModelIndex &parent)
{
    if(sourceModel())
    {
        QModelIndex iprnt = mapFromSource(parent);

        if(iprnt == QModelIndex() && parent != QModelIndex())
            return;

        int col_cnt = sourceModel()->columnCount(parent);
        for(int i = 0; i < col_cnt; ++i)
        {
            int irow_cnt = rowCount(iprnt);
            QModelIndex ecur = sourceModel()->index(row,i,parent);
            QModelIndex icur = createIndex(irow_cnt,columnCount(iprnt),(qint32)ecur.internalId());
            indexes[iprnt][irow_cnt][i] = icur;
        }
    }
}

void EFilterProxyModel::deleteRow(int row, const QModelIndex &parent)
{
    if(sourceModel())
    {
        QModelIndex iprnt = mapFromSource(parent);
        if(iprnt == QModelIndex() && parent != QModelIndex())
            return;

        if(row == sourceModel()->rowCount(parent) - 1)
            indexes[iprnt].remove(row);
        else if(row < sourceModel()->rowCount(parent))
        {
            int last_row = sourceModel()->rowCount(parent) - 1;
            for(int i = row; i < last_row; ++i)
                indexes[iprnt][i] = indexes[iprnt][i+1];
            indexes[iprnt].remove(last_row);
        }
    }
}
