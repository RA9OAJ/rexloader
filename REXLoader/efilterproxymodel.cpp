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
    if(sourceModel())
    {
        QModelIndex eprnt = mapToSource(parent);
        return sourceModel()->insertColumns(column,count,eprnt);
    }

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
            QModelIndex *iprnt = _srcmap.value(sourceIndex.parent(),0);
            QModelIndex iparent = (iprnt == 0 ? QModelIndex() : *iprnt);

            if(sourceIndex.parent() != QModelIndex() && iparent == QModelIndex())
                return cindex;

            int irow = indexes.value(iparent).indexOf(sourceIndex.row());

            if(irow != -1)
                cindex = createIndex(irow,sourceIndex.column(),
                                     (void*)_srcmap.value(sourceIndex.parent(),0));
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
        QModelIndex *iprnt = (QModelIndex*)child.internalPointer();
        if(iprnt)
            prnt = *iprnt;
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
    if(sourceModel())
    {
        QModelIndex eprnt = mapToSource(parent);

        if(_filters.isEmpty())
            return sourceModel()->removeRows(row,count,eprnt);

        QList<int> erows;

        for(int i = row; i < row + count; ++i)
        {
            int ercur = indexes.value(parent).value(i);

            if(erows.isEmpty())
            {
                erows.append(ercur);
                continue;
            }

            if(ercur < erows.first())
                erows.insert(0,ercur);
            else if(ercur > erows.last())
                erows.append(ercur);
            else
            { //двоичный поиск места вставки
                int first = 0;
                int last = erows.size() - 1;

                while(first < last - 1)
                {
                    int cntr = first + (last - first) / 2;

                    if(ercur > erows.value(cntr))
                        first = cntr;
                    else last = cntr;
                }
                erows.insert(last,ercur);
            }
        }

        int start;
        QList<int>::iterator i;
        for(i = erows.begin(), start = *i; i != erows.end(); ++i)
        {
            if(*i != *(i + 1) - 1)
            {
                sourceModel()->removeRows(start, *i - start, eprnt);
                start = *(i + 1);
            }
        }
    }

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

QList<EFFilter> EFilterProxyModel::filters(int key_column) const
{
    return _filters.values(key_column);
}

QList<EFFilter> EFilterProxyModel::getAllFilters() const
{
    return _filters.values();
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
    /*if(list.isEmpty())
        return;*/

    foreach (EFFilter fltr, list) {
        if (fltr == cur)
            return;
    }

    beginResetModel();
    _filters.insertMulti(key_column,cur);
    runFiltering();
    endResetModel();
}

void EFilterProxyModel::deleteFilter(int key_column)
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

void EFilterProxyModel::prepareFilter(int key_column, int filter_role, int _operator_, const QVariant &filter_val)
{
    EFFilter cur;
    cur.data_role = filter_role;
    cur.filter_operator = _operator_;
    cur.filter_value = filter_val;

    QList<EFFilter> list = _prepared.values(filter_role);

    foreach (EFFilter fltr, list) {
        if (fltr == cur)
            return;
    }

    _prepared.insertMulti(key_column,cur);
}

void EFilterProxyModel::prepareToRemoveFilter(int key_column)
{
    _for_remove.append(key_column);
}

void EFilterProxyModel::execPrepared()
{
    bool need_clean = !_for_remove.isEmpty();

    foreach (int col, _for_remove) //удаляем запланированные для удаления филтры
        _filters.remove(col);

    _for_remove.clear();

    bool need_filtering = !_prepared.isEmpty();
    _filters += _prepared;
    _prepared.clear();

    beginResetModel();
    /*if(need_clean) //услови для будущей оптимизации процесса фильтрации при добавлении нового условия
    {*/
        indexes.clear();
        clearSrcMap();
    //}
    if(need_filtering)
        runFiltering();
    endResetModel();
}

void EFilterProxyModel::proxyDataChanget(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if(_filters.isEmpty())
    {
        emit dataChanged(mapFromSource(topLeft),
                         mapFromSource(bottomRight));
        return;
    }

    if(topLeft == QModelIndex() || bottomRight == QModelIndex())
        return;

    int tLeft = -1, bRight = -1;
    QList<int> fltr_columns = _filters.keys();
    bool filter_col = false;

    for(int i = topLeft.column(); i <= bottomRight.column(); ++i)
        if(fltr_columns.indexOf(i) != -1)
        {
            filter_col = true;
            break;
        }

    for(int i = topLeft.row(); i <= bottomRight.row(); ++i) //проходим каждую строку
    {
        QModelIndex eidx = sourceModel()->index(i,0,topLeft.parent());
        bool internal = false;
        if(eidx != QModelIndex() && mapFromSource(eidx) != QModelIndex()) //присутствие во внутренней модели
            internal = true;

        if(filter_col) //если нужно проверить фильтры после изменения
        {
            if(matchFilters(i,topLeft.parent()))
            {
                if(!internal)
                    addRow(i,eidx.parent()); //добавляем из внешней во внутреннюю модель строку
                else
                {
                    if(tLeft == -1)
                        tLeft = eidx.row();
                    bRight = eidx.row();
                }
            }
            else if(internal)
            {
                deleteRow(i,eidx.parent()); //удаляем строку, пересавшую удовлетворять фильтрам
            }
        }
        else if(internal) //если строка есть во внутренней модели
        {
            if(tLeft == -1)
                tLeft = eidx.row();
            bRight = eidx.row();
        }
    }

    if(tLeft != -1 && bRight != -1)
    {
        QModelIndex itopLeft = mapFromSource(sourceModel()->index(tLeft,
                                                                  topLeft.column(),
                                                                  topLeft.parent()));
        QModelIndex ibottomRight = mapFromSource(sourceModel()->index(bRight,
                                                                      bottomRight.column(),
                                                                      bottomRight.parent()));

        emit dataChanged(itopLeft,ibottomRight);
    }
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
    QModelIndex iparent = mapFromSource(parent);
    if(parent != QModelIndex() && iparent == QModelIndex())
        return;

    if(_filters.isEmpty())
    {
        /*row_diff = end - start + 1;

        beginInsertRows(iparent,start,end);*/
        removeSrcMap(parent);
        runFiltering(0,parent);
        /*row_diff = 0;
        endInsertRows();*/
    }
    else
    {
        //Реакция на смену номеров строк во внешней модели
        int first = 0;
        int last = indexes.value(iparent).size() - 1;
        while(first < last - 1)
        {
            int cntr = first + (last - first) / 2;

            if(start <= indexes.value(iparent).value(cntr))
                first = cntr;
            else last = cntr;
        }
        for(int i = first; i < indexes.value(iparent).size(); ++i)
            indexes[iparent][i] = indexes.value(iparent).value(i) + end - start;

        runFiltering(start,parent,end - start + 1);
    }
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
    for(int i = start; i <= end; ++i)
    {
        QModelIndex eidx = sourceModel()->index(start,0,parent);
        if(eidx != QModelIndex())
            deleteRow(i,parent);
    }
}

void EFilterProxyModel::runSorting(int column, Qt::SortOrder order)
{
    Q_UNUSED(column);
    Q_UNUSED(order);
}

bool EFilterProxyModel::runFiltering(int row, const QModelIndex &parent, int row_count)
{
    int cnt = 0;
    if(sourceModel())
    {
        if(!_filters.isEmpty())
        {
            int end = (row_count < 0 ? sourceModel()->rowCount(parent) : row + row_count);
            for(int i = row; i < end; ++i)
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
                int end = (row_count < 0 ? sourceModel()->rowCount(parent) : row + row_count);
                for(int y = 0; y < end; ++y)
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
    indexes.clear();
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
                {
                    QString cdata = cur_idx.data(fltr.data_role).toString();
                    QString pattern = QRegExp::escape(fltr.filter_value.toString());
                    pattern = QString("^%1$").arg(pattern.replace("%",".*"));
                    QRegExp regexp(pattern,Qt::CaseInsensitive);
                    result = (cdata.indexOf(regexp) == -1 ? false : true);
                    break;
                }
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
                    result = equal(cur_idx.data(fltr.data_role), fltr.filter_value);
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
        QModelIndex eidx = sourceModel()->index(row,0,parent);

        if(parent == QModelIndex())
        {
            beginInsertRows(iprnt,rowCount(iprnt),rowCount(iprnt));
            indexes[iprnt].append(row);
            endInsertRows();
            return;
        }

        QList<QPair<QModelIndex,int> > row_list;

        while(eidx.parent() != QModelIndex() && iprnt == QModelIndex())
        {
            QPair<QModelIndex, int> cur_lvl;
            cur_lvl.first = eidx.parent();
            cur_lvl.second = eidx.row();
            row_list.prepend(cur_lvl);

            eidx = eidx.parent();
            iprnt = mapFromSource(eidx.parent());
        }

        QList<QPair<QModelIndex,int> >::iterator i = row_list.begin();
        while(i != row_list.end())
        {
            QModelIndex eprnt = (*i).first.parent();
            iprnt = mapFromSource(eprnt);

            beginInsertRows(iprnt,rowCount(iprnt),rowCount(iprnt));
            if(eprnt != QModelIndex() && !_srcmap.contains(eprnt))
                _srcmap.insert(parent,new QModelIndex(iprnt));

            indexes[iprnt].append(row);
            endInsertRows();

            ++i;
        }
    }
}

void EFilterProxyModel::deleteRow(int row, const QModelIndex &parent, bool deep)
{
    if(sourceModel())
    {
        QModelIndex iprnt = mapFromSource(parent);
        if(iprnt == QModelIndex() && parent != QModelIndex())
            return;

        if(deep) //удалять по восходящей всех родителей, не удовлетворяющих фильтрам
        {
            deleteRow(row,parent);

            QModelIndex newparent = parent;
            int newrow = row;

            do
            {
                newrow = newparent.row();
                newparent = newparent.parent();

                if(matchFilters(newrow,newparent)) //если это строка удовлетворяет фильтрам, то выходим
                   break;

                deleteRow(newrow,newparent); //иначе удаляем строку
            }while(newparent != QModelIndex());
        }
        else //удалить только потомков и саму строку
        {
            int irkey = indexes.value(iprnt).indexOf(row);
            if(irkey != -1)
            {
                QModelIndex neweprnt = sourceModel()->index(row,0,parent); //определяем нового внешнего родителя
                QModelIndex newiprnt = mapFromSource(neweprnt); //определяем нового внутреннего родителя
                if(newiprnt != QModelIndex()) //если все корректно преобразовалось
                    for(int i = rowCount(newiprnt) - 1; i >= 0; --i) //в цикле перебираем все строки
                        if(hasChildren(index(i,0,newiprnt))) //проверяем, имеет ли строка потомков
                        {   //если есть потомки
                            QModelIndex eidx = sourceModel()->index(i,0,neweprnt);
                            deleteRow(eidx.row(),eidx.parent()); //вызываем удаление для потомков
                        }

                indexes[iprnt].removeAt(irkey); //удаляем строку
            }

            if(!rowCount(iprnt))
            {
                //если у внутреннего родителя нет потомков, то удаляем его из списка родителей
                indexes.remove(iprnt);
                removeSrcMap(parent);
            }
        }
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
    switch (val1.type()) {
    case QVariant::UInt:
    case QVariant::ULongLong:
        return val1.toULongLong() > val2.toULongLong();

    case QVariant::Double:
    case QVariant::SizeF:
        return val1.toDouble() > val2.toDouble();

    case QVariant::DateTime:
        if(val2.type() == QVariant::DateTime)
            return val1.toDateTime() > val2.toDateTime();
    case QVariant::Date:
        return val1.toDate() > val2.toDate();

    case QVariant::Int:
    case QVariant::Bool:
    case QVariant::LongLong:
    case QVariant::Size:
        return val1.toLongLong() > val2.toLongLong();

    default:
        return false;
    }
}


bool operator <(const QVariant &val1, const QVariant &val2)
{
    switch (val1.type()) {
    case QVariant::UInt:
    case QVariant::ULongLong:
        return val1.toULongLong() < val2.toULongLong();

    case QVariant::Double:
    case QVariant::SizeF:
        return val1.toDouble() < val2.toDouble();

    case QVariant::DateTime:
        if(val2.type() == QVariant::DateTime)
            return val1.toDateTime() < val2.toDateTime();
    case QVariant::Date:
        return val1.toDate() < val2.toDate();

    case QVariant::Int:
    case QVariant::Bool:
    case QVariant::LongLong:
    case QVariant::Size:
        return val1.toLongLong() < val2.toLongLong();

    default:
        return false;
    }
}


bool operator <=(const QVariant &val1, const QVariant &val2)
{
    switch (val1.type()) {
    case QVariant::UInt:
    case QVariant::ULongLong:
        return val1.toULongLong() <= val2.toULongLong();

    case QVariant::Double:
    case QVariant::SizeF:
        return val1.toDouble() <= val2.toDouble();

    case QVariant::DateTime:
        if(val2.type() == QVariant::DateTime)
            return val1.toDateTime() <= val2.toDateTime();
    case QVariant::Date:
        return val1.toDate() <= val2.toDate();

    case QVariant::Int:
    case QVariant::Bool:
    case QVariant::LongLong:
    case QVariant::Size:
        return val1.toLongLong() <= val2.toLongLong();

    default:
        return false;
    }
}


bool operator >=(const QVariant &val1, const QVariant &val2)
{
    switch (val1.type()) {
    case QVariant::UInt:
    case QVariant::ULongLong:
        return val1.toULongLong() >= val2.toULongLong();

    case QVariant::Double:
    case QVariant::SizeF:
        return val1.toDouble() >= val2.toDouble();

    case QVariant::DateTime:
        if(val2.type() == QVariant::DateTime)
            return val1.toDateTime() >= val2.toDateTime();
    case QVariant::Date:
        return val1.toDate() >= val2.toDate();

    case QVariant::Int:
    case QVariant::Bool:
    case QVariant::LongLong:
    case QVariant::Size:
        return val1.toLongLong() >= val2.toLongLong();

    default:
        return false;
    }
}


bool equal(const QVariant &val1, const QVariant &val2)
{
    switch (val1.type()) {
    case QVariant::DateTime:
        if(val2.type() == QVariant::DateTime)
            return val1.toDateTime() == val2.toDateTime();
    case QVariant::Date:
        return val1.toDate() == val2.toDate();

    default:
        return val1 == val2;
    }
}


bool EFilterProxyModel::containsFilter(int key_column, int filter_role, int _operator_, const QVariant &filter_val) const
{
    EFFilter fltr;
    fltr.data_role = filter_role;
    fltr.filter_operator = _operator_;
    fltr.filter_value = filter_val;

    return _filters.contains(key_column,fltr);
}
