#include "efilterproxymodel.h"

EFilterProxyModel::EFilterProxyModel(QObject *parent) :
    QAbstractProxyModel(parent)
{
    setParent(parent);
    row_diff = 0;
    _src = 0;
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
    if(sourceModel())
       return sourceModel()->canFetchMore(mapToSource(parent));

    return false;
}

int EFilterProxyModel::columnCount(const QModelIndex &parent) const
{
    if(sourceModel())
        return sourceModel()->columnCount(mapToSource(parent));

    return 0;
}

QVariant EFilterProxyModel::data(const QModelIndex &index, int role) const
{
    if(sourceModel())
        return sourceModel()->data(mapToSource(index),role);
    return QVariant();
}

void EFilterProxyModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent);
}

Qt::ItemFlags EFilterProxyModel::flags(const QModelIndex &index) const
{
    if(sourceModel())
        return sourceModel()->flags(mapToSource(index));

    return Qt::NoItemFlags;
}

bool EFilterProxyModel::hasChildren(const QModelIndex &parent) const
{
    if(sourceModel())
        if(_filters.isEmpty())
            return sourceModel()->hasChildren(mapToSource(parent));

    if(parent == QModelIndex())
        return !indexes.isEmpty();

    QModelIndex eprnt = mapToSource(parent);
    return _srcmap.contains(eprnt);
}

QVariant EFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(sourceModel())
        return sourceModel()->headerData(section,orientation,role);

    return QVariant();
}

QModelIndex EFilterProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if(sourceModel() && row >= 0 && column >= 0)
    {
        if(_filters.isEmpty())
        {
            QModelIndex eidx = mapToSource(parent);
            const QModelIndex *iprnt = _srcmap.value(eidx,0);
            eidx = sourceModel()->index(row,column,eidx);
            QModelIndex iidx = createIndex(row,column,(void*)iprnt);
            return iidx;
        }

        if(hasIndex(row,column,parent))
        {
            QModelIndex eprnt = mapToSource(parent);
            QModelIndex iindex = createIndex(row,column,(void*)_srcmap.value(eprnt,0));
            return iindex;
        }
    }
    return QModelIndex();
}

bool EFilterProxyModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(column);
    Q_UNUSED(count);
    Q_UNUSED(parent);

    return false;
}

bool EFilterProxyModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if(sourceModel())
    {
        QModelIndex eprnt = mapToSource(parent);
        if(parent != QModelIndex() && eprnt == QModelIndex())
            return false;

        int erow = 0;
        if(row <= columnCount(parent))
        {
            QModelIndex eidx;
            if(row < columnCount(parent))
            {
                eidx = mapToSource(index(row,0,parent));
                erow = eidx.row();
            }
            else
            {
                eidx = mapToSource(index(row-1,0,parent));
                if(eidx.row() < sourceModel()->rowCount(eprnt) - 1)
                {
                    eidx = sourceModel()->index(eidx.row()+1,0,eprnt);
                    erow = eidx.row();
                }
                else erow = sourceModel()->rowCount(eprnt);
            }
        }
        else return false;

        return sourceModel()->insertRows(erow,count,eprnt);
    }

    return false;
}

QModelIndex EFilterProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    QModelIndex cindex;
    if(sourceModel() && sourceIndex != QModelIndex())
    {
        if(_filters.isEmpty())
        {
            QModelIndex *iprnt = _srcmap.value(sourceIndex.parent(),0);
            if(sourceIndex.parent() != QModelIndex() && iprnt)
                return cindex;
            cindex = createIndex(sourceIndex.row(),
                                 sourceIndex.column(),
                                 (void*)iprnt);
        }
        else if(!indexes.isEmpty())
        {
            QModelIndex iprnt = *(_srcmap.value(sourceIndex.parent()));
            if(sourceIndex.parent() != QModelIndex() && iprnt == QModelIndex())
                return cindex;

            int irow = indexes.value(iprnt).indexOf(sourceIndex.row());

            if(irow != -1)
                cindex = createIndex(irow,sourceIndex.column(),
                                     (void*)_srcmap.value(sourceIndex.parent()));
        }
    }
    return cindex;
}

QItemSelection EFilterProxyModel::mapSelectionFromSource(const QItemSelection &sourceSelection) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
        {
            return sourceSelection;
        }
        //Доработать
    }

    return QItemSelection();
}

QItemSelection EFilterProxyModel::mapSelectionToSource(const QItemSelection &proxySelection) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
        {
            return proxySelection;
        }
        //Доработать
    }

    return QItemSelection();
}

QModelIndex EFilterProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    QModelIndex eindex;
    if(sourceModel() && proxyIndex != QModelIndex())
    {
        if(_filters.isEmpty())
        {
            QModelIndex *idx = static_cast<QModelIndex*>(proxyIndex.internalPointer());

            eindex = sourceModel()->index(proxyIndex.row(),
                                          proxyIndex.column(),
                                          _srcmap.key(idx,QModelIndex()));
        }
        else if(!indexes.isEmpty())
        {
            eindex = sourceModel()->index(indexes.value(proxyIndex.parent()).value(proxyIndex.row()),
                                          proxyIndex.column(),
                                          _srcmap.key((QModelIndex*)proxyIndex.internalPointer(),QModelIndex()));
        }
    }
    return eindex;
}

QModelIndexList EFilterProxyModel::match(const QModelIndex &start, int role, const QVariant &value, int hits, Qt::MatchFlags flags) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
            return sourceModel()->match(mapToSource(start),role,value,hits,flags);
        //Доработать
    }
    return QModelIndexList();
}

QMimeData *EFilterProxyModel::mimeData(const QModelIndexList &indexes) const
{
    if(sourceModel())
    {
        QModelIndexList emdllist;
        foreach (QModelIndex cur, indexes) {
            QModelIndex eidx = mapToSource(cur);
            if(eidx != QModelIndex())
                emdllist.append(eidx);
        }
        return sourceModel()->mimeData(emdllist);
    }

    return 0;
}

QStringList EFilterProxyModel::mimeTypes() const
{
    if(sourceModel())
        return sourceModel()->mimeTypes();

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
        //beginRemoveColumns(parent,column,column + count - 1);
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
            runFiltering(0,parent);

        //endRemoveColumns();
    }
    return false;
}

bool EFilterProxyModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(count);
    Q_UNUSED(parent);

    return false;
}

int EFilterProxyModel::rowCount(const QModelIndex &parent) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
            return sourceModel()->rowCount(mapToSource(parent)) - row_diff;
    }
    int cnt = 0;
    if(indexes.contains(parent))
        cnt = indexes.value(parent).size();

    return cnt;
}

bool EFilterProxyModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(sourceModel())
        return sourceModel()->setData(mapToSource(index),value, role);

    return false;
}

bool EFilterProxyModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if(sourceModel())
    {
        if(orientation == Qt::Horizontal)
            return sourceModel()->setHeaderData(section,orientation,value,role);

        //Доработать в части вртикальных заголовков
    }
    return false;
}

void EFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if(_src)
    {
        disconnect(_src,SIGNAL(modelReset()),this,SLOT(proxyModelReset()));
        disconnect(_src,SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(proxyDataChanget(QModelIndex,QModelIndex)));
        disconnect(_src,SIGNAL(headerDataChanged(Qt::Orientation,int,int)),this,SLOT(proxyHeaderDataChanget(Qt::Orientation,int,int)));
        disconnect(_src,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(proxyRowsInsrted(QModelIndex,int,int)));
        disconnect(_src,SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),this,SLOT(proxyRowsMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(_src,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(proxyRowsRemoved(QModelIndex,int,int)));
    }

    _src = sourceModel;
    connect(_src,SIGNAL(modelReset()),SLOT(proxyModelReset()));
    connect(_src,SIGNAL(dataChanged(QModelIndex,QModelIndex)),SLOT(proxyDataChanget(QModelIndex,QModelIndex)));
    connect(_src,SIGNAL(headerDataChanged(Qt::Orientation,int,int)),SLOT(proxyHeaderDataChanget(Qt::Orientation,int,int)));
    connect(_src,SIGNAL(rowsInserted(QModelIndex,int,int)),SLOT(proxyRowsInsrted(QModelIndex,int,int)));
    connect(_src,SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)),SLOT(proxyRowsMoved(QModelIndex,int,int,QModelIndex,int)));
    connect(_src,SIGNAL(rowsRemoved(QModelIndex,int,int)),SLOT(proxyRowsRemoved(QModelIndex,int,int)));
    reset();
}

void EFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    _sort_param.first = column;
    _sort_param.second = order;
}

bool EFilterProxyModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
            return hasIndex(row,column,mapToSource(parent));

        if(indexes.contains(parent) && row < indexes.value(parent).size())
        {
            QModelIndex eprnt = index(parent.row(),parent.column(),parent.parent());
            eprnt = _srcmap.key((QModelIndex*)eprnt.internalPointer(),QModelIndex());
            if(column < sourceModel()->columnCount(eprnt))
                return true;
        }
    }
    return false;
}

QSize EFilterProxyModel::span(const QModelIndex &index) const
{
    if(sourceModel())
    {
        if(_filters.isEmpty())
            return sourceModel()->span(mapToSource(index));
    }
     //Доработать объединение ячеек (трансляцию) для внутренней модели

    return QSize();
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

    beginResetModel();
    _filters.insertMulti(key_column,cur);
    runFiltering();
    endResetModel();
}

void EFilterProxyModel::clearFilter(int key_column)
{
    beginResetModel();
    _filters.remove(key_column);
    runFiltering();
    endResetModel();
    //Доработать реакцию на смену фильтров + сортировка
}

void EFilterProxyModel::clearAllFilters()
{
    beginResetModel();
    _filters.clear();
    clearSrcMap();
    indexes.clear();
    endResetModel();

}

void EFilterProxyModel::proxyDataChanget(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    emit dataChanged(mapFromSource(topLeft),
                     mapFromSource(bottomRight));
}

void EFilterProxyModel::proxyHeaderDataChanget(Qt::Orientation orientation, int first, int last)
{
    emit headerDataChanged(orientation,first,last);
}

void EFilterProxyModel::proxyModelReset()
{
    reset();
}

void EFilterProxyModel::proxyRowsInsrted(const QModelIndex &parent, int start, int end)
{
    row_diff = end - start + 1;
    beginInsertRows(mapFromSource(parent),start,end);
    removeSrcMap(parent);
    runFiltering(0,parent);
    row_diff = 0;
    endInsertRows();
}

void EFilterProxyModel::proxyRowsMoved(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
{
    Q_UNUSED(sourceParent);
    Q_UNUSED(sourceStart);
    Q_UNUSED(sourceEnd);
    Q_UNUSED(destinationParent);
    Q_UNUSED(destinationRow);
}

void EFilterProxyModel::proxyRowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);
}

void EFilterProxyModel::runSorting(int column, Qt::SortOrder order)
{
    Q_UNUSED(column);
    Q_UNUSED(order);
}

bool EFilterProxyModel::runFiltering(int row, const QModelIndex &parent)
{
    int cnt = 0;
    if(sourceModel())
    {
        if(!_filters.isEmpty())
        {
            for(int i = row; row < sourceModel()->rowCount(parent); ++i)
            {
                QModelIndex nparent = sourceModel()->index(i,0,parent);
                if(sourceModel()->rowCount(nparent) && nparent != QModelIndex())
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
        else
        {
            if(!_srcmap.contains(parent) && parent != QModelIndex()) //если нет родителя, то заносим его
            {
                QModelIndex iprnt = createIndex(parent.row(),parent.column(),(void*)_srcmap.value(parent,0));
                insertSrcMap(parent,iprnt);
            }

            for(int i = row; i < sourceModel()->rowCount(parent); ++i) //перебираем все строки
            {//перебираем все элементы строки и ищем элементы-родители
                for(int y = 0; y < sourceModel()->columnCount(parent); ++y)
                {
                    QModelIndex eidx = sourceModel()->index(i,y,parent);
                    if(sourceModel()->hasChildren(eidx))
                        runFiltering(0,eidx); //вызываем фильтрацию для элемента-родителя
                }
            }
        }
    }
    return cnt;
}

void EFilterProxyModel::reset()
{
    beginResetModel();
    _filters.clear();
    clearSrcMap();
    runFiltering();
    endResetModel();
}

bool EFilterProxyModel::matchFilters(int row, const QModelIndex &parent) const
{
    bool result = false;
    if(sourceModel())
    {
        foreach (int col, _filters.keys())
        {
            QModelIndex cur_idx = sourceModel()->index(row,col,parent);
            QList<EFFilter> fltrs = _filters.values(col);
            foreach (EFFilter fltr, fltrs)
            {
                QVariant val = cur_idx.data(fltr.data_role);

                int cur_oper = fltr.filter_operator & 0xfd;
                bool not_flag = fltr.filter_operator & 0x2;
                bool equal_flag = fltr.filter_operator & 0x1;

                switch (cur_oper)
                {
                case Like:
                    break;
                case Larger:
                    result = (equal_flag ? (cur_idx.data(fltr.data_role) >= fltr.filter_value)
                                         : (cur_idx.data(fltr.data_role) > fltr.filter_value));
                    break;
                case Lesser:
                    result = (equal_flag ? (cur_idx.data(fltr.data_role) <= fltr.filter_value)
                                         : (cur_idx.data(fltr.data_role) < fltr.filter_value));
                    break;
                case In:
                    result = false;
                    break;
                case Between:
                    result = false;
                    break;
                case Equal:
                    result = (cur_idx.data(fltr.data_role) == fltr.filter_value);
                    break;

                default:
                    result = false;
                    break;
                }
                if(fltr.filter_operator & 0x65 && not_flag)
                    result = !result;

                if(!result)
                    break;
            }
            if(!result)
                break;
        }
    }
    return result;
}

void EFilterProxyModel::addRow(int row, const QModelIndex &parent)
{
    if(sourceModel())
    {
        QModelIndex iprnt = mapFromSource(parent);

        if(iprnt == QModelIndex() && parent != QModelIndex())
            return;

        indexes[iprnt].append(row);
    }
}

void EFilterProxyModel::deleteRow(int row, const QModelIndex &parent)
{
    if(sourceModel())
    {
        QModelIndex iprnt = mapFromSource(parent);
        if(iprnt == QModelIndex() && parent != QModelIndex())
            return;

        int irkey = indexes.value(iprnt).indexOf(row);
        if(irkey != -1)
            indexes[iprnt].removeAt(irkey);
    }
}

void EFilterProxyModel::removeSrcMap(const QModelIndex &key)
{
    QModelIndex *iidx = _srcmap.value(key,0);
    if(iidx)
        delete iidx;
    _srcmap.remove(key);
}

void EFilterProxyModel::insertSrcMap(const QModelIndex &key, const QModelIndex &val)
{
    QModelIndex * iidx = _srcmap.value(key,0);
    if(iidx)
        delete iidx;
    _srcmap.insert(key,new QModelIndex(val));
}

void EFilterProxyModel::clearSrcMap()
{
    QList<QModelIndex*> lst = _srcmap.values();
    _srcmap.clear();

    foreach (QModelIndex *iidx, lst)
        delete iidx;
}


bool operator >(const QVariant &val1, const QVariant &val2)
{
    int digits = QVariant::Int | QVariant::Bool | QVariant::LongLong | QVariant::Size;
    int unsigned_digits =  QVariant::UInt | QVariant::ULongLong;
    int float_digits = QVariant::Double | QVariant::SizeF;
    int date = QVariant::Date | QVariant::DateTime;

    if(val1.type() & digits)
        return val1.toLongLong() > val2.toLongLong();

    else if(val1.type() & unsigned_digits)
        return val1.toULongLong() > val2.toULongLong();

    else if(val1.type() & float_digits)
        return val1.toDouble() > val2.toDouble();

    else if(val1.type() & date)
        return val1.toDateTime() > val2.toDateTime();

    return false;
}


bool operator <(const QVariant &val1, const QVariant &val2)
{
    int digits = QVariant::Int | QVariant::Bool | QVariant::LongLong | QVariant::Size;
    int unsigned_digits =  QVariant::UInt | QVariant::ULongLong;
    int float_digits = QVariant::Double | QVariant::SizeF;
    int date = QVariant::Date | QVariant::DateTime;

    if(val1.type() & digits)
        return val1.toLongLong() < val2.toLongLong();

    else if(val1.type() & unsigned_digits)
        return val1.toULongLong() < val2.toULongLong();

    else if(val1.type() & float_digits)
        return val1.toDouble() < val2.toDouble();

    else if(val1.type() & date)
        return val1.toDateTime() < val2.toDateTime();

    return false;
}


bool operator <=(const QVariant &val1, const QVariant &val2)
{
    int digits = QVariant::Int | QVariant::Bool | QVariant::LongLong | QVariant::Size;
    int unsigned_digits =  QVariant::UInt | QVariant::ULongLong;
    int float_digits = QVariant::Double | QVariant::SizeF;
    int date = QVariant::Date | QVariant::DateTime;

    if(val1.type() & digits)
        return val1.toLongLong() <= val2.toLongLong();

    else if(val1.type() & unsigned_digits)
        return val1.toULongLong() <= val2.toULongLong();

    else if(val1.type() & float_digits)
        return val1.toDouble() <= val2.toDouble();

    else if(val1.type() & date)
        return val1.toDateTime() <= val2.toDateTime();

    return false;
}


bool operator >=(const QVariant &val1, const QVariant &val2)
{
    int digits = QVariant::Int | QVariant::Bool | QVariant::LongLong | QVariant::Size;
    int unsigned_digits =  QVariant::UInt | QVariant::ULongLong;
    int float_digits = QVariant::Double | QVariant::SizeF;
    int date = QVariant::Date | QVariant::DateTime;

    if(val1.type() & digits)
        return val1.toLongLong() >= val2.toLongLong();

    else if(val1.type() & unsigned_digits)
        return val1.toULongLong() >= val2.toULongLong();

    else if(val1.type() & float_digits)
        return val1.toDouble() >= val2.toDouble();

    else if(val1.type() & date)
        return val1.toDateTime() >= val2.toDateTime();

    return false;
}
