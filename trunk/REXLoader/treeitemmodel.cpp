/*
Project: REXLoader (Downloader), Source file: treeitemmodel.cpp
Copyright (C) 2011  Sarvaritdinov R.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "treeitemmodel.h"

TreeItemModel::TreeItemModel(QObject *parent) :
    QAbstractItemModel(parent)
{
    gcol = 0;
    qr = 0;
    nodes.insert(QModelIndex(),QVariant());
    font.setPointSize(10);
    ignore_flag = false;
}

TreeItemModel::~TreeItemModel()
{
    if(qr)delete(qr);
}

QVariant TreeItemModel::data(const QModelIndex &index, int role) const
{
    switch(role)
    {
    case Qt::DisplayRole:
        if(!index.column())
        {
            QString nodename;
            if(nodes.value(index).toString() == "#downloads") nodename = tr("Все закачки");
            else if(nodes.value(index).toString() == "#archives") nodename = tr("Архивы");
            else if(nodes.value(index).toString() == "#apps") nodename = tr("Приложения");
            else if(nodes.value(index).toString() == "#audio") nodename = tr("Аудио");
            else if(nodes.value(index).toString() == "#video") nodename = tr("Видео");
            else if(nodes.value(index).toString() == "#other") nodename = tr("Другое");
            else nodename = nodes.value(index).toString();

            int tsk_cnt = taskCount(index);
            int incompl_cnt = taskCount(index, true);
            if(tsk_cnt) incompl_cnt ? nodename += QString(" (%1/%2)").arg(QString::number(incompl_cnt),QString::number(tsk_cnt)) : nodename += QString(" (%1)").arg(QString::number(tsk_cnt));

            return nodename;
        }
        return nodes.value(index);

    case Qt::FontRole:
        if(!index.column())
        {
            if(nodes.value(index).toString() != "#downloads" && taskCount(index, true))
            {
                QFont fnt(font);
                fnt.setWeight((int)QFont::DemiBold);
                return fnt;
            }
            return font;
        }
        return QFont();

    case Qt::EditRole:
    case 100: return nodes.value(index);
    case 101:
        if(!index.column())
        {
            QString nodename;
            if(nodes.value(index).toString() == "#downloads") nodename = tr("Все закачки");
            else if(nodes.value(index).toString() == "#archives") nodename = tr("Архивы");
            else if(nodes.value(index).toString() == "#apps") nodename = tr("Приложения");
            else if(nodes.value(index).toString() == "#audio") nodename = tr("Аудио");
            else if(nodes.value(index).toString() == "#video") nodename = tr("Видео");
            else if(nodes.value(index).toString() == "#other") nodename = tr("Другое");
            else nodename = nodes.value(index).toString();

            return nodename;
        }

    default: return QVariant();
    }
}

int TreeItemModel::rowCount(const QModelIndex &parent) const
{
    QList<QModelIndex> cur_nodes = link.keys(parent);
    if(!cur_nodes.size())return 0;
    int col_row = 0;
    for(int i = 0; i < cur_nodes.size(); ++i)
        if(!cur_nodes.value(i).column())col_row++;
    return col_row;
}

int TreeItemModel::columnCount(const QModelIndex &parent) const
{
    //if(QModelIndex() == parent)return 1;
    QList<QModelIndex> children = link.keys(parent);
    if(!children.size()) return 0;
    int max_col = 0;
    for(int i = 0; i < children.size(); ++i)
        max_col = qMax(children.value(i).column(), max_col);
    return max_col+1;
}

QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if(column > columnCount(parent) || row > rowCount(parent) /*|| link.key(parent, QModelIndex()) == QModelIndex()*/)return QModelIndex();
    QList<QModelIndex> cur_nodes = link.keys(parent);
    if(!cur_nodes.size()) return QModelIndex();
    QModelIndex cur = QModelIndex();
    for(int i = 0; i < cur_nodes.size(); ++i)
        if(cur_nodes.value(i).row() == row && cur_nodes.value(i).column() == column) cur = cur_nodes.value(i);
    return cur;
}

QModelIndex TreeItemModel::parent(const QModelIndex &child) const
{
    if(!link.contains(child)) return QModelIndex();
    return link.value(child);
}

bool TreeItemModel::updateModel(const QSqlDatabase &db)
{
    if(qr)delete(qr);
    qr = 0;
    qr = new QSqlQuery("SELECT title, id, dir, extlist, parent_id FROM categories ORDER BY parent_id ASC",db);
    if(!qr->exec())
    {
        reset();
        addFiltersSubtree();
        return false;
    }

    nodes.clear();
    link.clear();
    nodes.insert(QModelIndex(),QVariant());
    QHash<int,QModelIndex> key_nodes;
    QHash<QModelIndex,int> row_cnt;
    row_cnt.insert(QModelIndex(),0);
    while(qr->next())
    {
        gcol = qr->record().count();
        QModelIndex parent;
        if(!qr->value(4).toInt()) parent = QModelIndex();
        else parent = key_nodes.value(qr->value(4).toInt());
        for(int i = 0; i < qr->record().count(); ++i)
        {
            QModelIndex cur = createIndex(row_cnt.value(parent),i,nodes.size());
            nodes.insert(cur,qr->value(i));
            link.insert(cur,parent);
            if(!i)
            {
                key_nodes.insert(qr->value(1).toInt(),cur);
            }
        }
        row_cnt[parent]++;
    }
    reset();
    delete(qr);
    qr = 0;
    if(!ignore_flag)addFiltersSubtree();
    return true;
}

bool TreeItemModel::hasChildren(const QModelIndex &parent) const
{
    if(link.keys(parent).size())return true;
    return false;
}

bool TreeItemModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    if(!nodes.contains(parent))return false;
    if(row > link.keys(parent).size() || column > columnCount(parent)) return false;
    return true;
}

void TreeItemModel::addFiltersSubtree()
{
    QList<QVariant> filters;
    filters << tr("Состояния")<< -1 << 0 << 0;
    filters << tr("Выполняются")<< -2 << -1 << 3;
    filters << tr("В ожидании") << -3 << -1 << -100;
    filters << tr("Остановленные") << -4 << -1 << 0;
    filters << tr("Завершенные") << -5 << -1 << 5;
    filters << tr("С ошибками") << -6 << -1 << -2;

    QModelIndex parent = QModelIndex();
    QHash<QModelIndex, int> cur_nodes;
    for(int i = 0; i < filters.size()/4; ++i)
    {
        if(!i)parent = QModelIndex();
        else parent = cur_nodes.key(filters.value(i*4+2).toInt());

        int row_cnt = i-1;
        if(!i)row_cnt = rowCount(parent);
        QModelIndex cur = createIndex(row_cnt,0,nodes.size());
        nodes.insert(cur,filters.value(i*4));
        link.insert(cur,parent);
        cur_nodes.insert(cur,filters.value(i*4+1).toInt());

        cur = createIndex(row_cnt,1,nodes.size());
        nodes.insert(cur,filters.value(i*4+1));
        link.insert(cur,parent);

        cur = createIndex(row_cnt,2,nodes.size());
        nodes.insert(cur,filters.value(i*4+2));
        link.insert(cur,parent);

        cur = createIndex(row_cnt,3,nodes.size());
        nodes.insert(cur,filters.value(i*4+3));
        link.insert(cur,parent);
    }
}

Qt::ItemFlags TreeItemModel::flags(const QModelIndex &index) const
{
    QModelIndex index_id = this->index(index.row(),1,index.parent());
    int id = data(index_id,100).toInt();
    if(id < 1) return QAbstractItemModel::flags(index);
    else if(id == 1) return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;
    return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QFont TreeItemModel::getFont() const
{
    return font;
}

void TreeItemModel::setFont(const QFont &fnt)
{
    font = fnt;
}

bool TreeItemModel::silentUpdate(const QSqlDatabase &db)
{
    if(qr)delete(qr);
    qr = 0;
    qr = new QSqlQuery("SELECT title, id, dir, extlist, parent_id FROM categories ORDER BY parent_id ASC",db);
    if(!qr->exec())
    {
        reset();
        addFiltersSubtree();
        return false;
    }

    nodes.clear();
    link.clear();
    nodes.insert(QModelIndex(),QVariant());
    QHash<int,QModelIndex> key_nodes;
    QHash<QModelIndex,int> row_cnt;
    row_cnt.insert(QModelIndex(),0);
    while(qr->next())
    {
        gcol = qr->record().count();
        QModelIndex parent;
        if(!qr->value(4).toInt()) parent = QModelIndex();
        else parent = key_nodes.value(qr->value(4).toInt());
        for(int i = 0; i < qr->record().count(); ++i)
        {
            QModelIndex cur = createIndex(row_cnt.value(parent),i,nodes.size());
            nodes.insert(cur,qr->value(i));
            link.insert(cur,parent);
            if(!i)
            {
                key_nodes.insert(qr->value(1).toInt(),cur);
            }
        }
        row_cnt[parent]++;
    }
    delete(qr);
    qr = 0;
    if(ignore_flag)addFiltersSubtree();
    return true;
}

void TreeItemModel::updateRow(int row, const QModelIndex &parent)
{

}

void TreeItemModel::updateRow(const QModelIndex &index)
{
    emit dataChanged(index,index);
}

QList<QModelIndex> TreeItemModel::parentsInTree() const
{
    QList<QModelIndex> list = link.values();
    for(int i = 0; i < list.size();)
    {
        int cnt = list.size();
        if(list.value(i) == QModelIndex())
        {
            list.removeAll(QModelIndex());
            continue;
        }
        while(list.count(list.value(i)) > 1)
            list.removeOne(list.value(i));
        if(cnt > list.size()) i = 0;
        else ++i;
    }
    return list;
}

bool TreeItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if(row >= rowCount(parent)) return false;
    if(count > rowCount(parent))return false;
    QModelIndex cur = QModelIndex();

    beginRemoveRows(parent, row, row + count - 1);
    for(int i = row; i < row + count; ++i)
    {
        for(int t = 0; t < columnCount(parent); ++t)
        {
            cur = index(i,t,parent); //определяем текущий элемент строки для удаления
            if(!t)
            {
                for(int y = 0; y < rowCount(cur); ++y) //перепривязываем детей удаляемой строки к родителю удаляемой строки
                {
                    for(int z = 0; z < columnCount(cur); ++z)
                    {
                        QModelIndex curchild = index(y,z,cur);
                        link.insert(curchild,parent);
                    }
                }
            }
            link.remove(cur);
            nodes.remove(cur);
        }
    }

    for(int i = row + count; index(i,0,parent) != QModelIndex(); i++)
    {
        for(int y = 0; y < gcol; y++)
        {
            cur = index(i,y,parent);
            if(!y)
            {
                QModelIndexList child_nodes = link.keys(cur);
                QModelIndex new_node = createIndex(i - 1, 0, row*gcol);
                nodes.insert(new_node, nodes.value(cur));
                link.insert(new_node,link.value(cur));
                nodes.remove(cur);
                link.remove(cur);
                for(int z = 0; z < child_nodes.size(); z++)
                    link.insert(child_nodes.value(z),new_node);
                continue;
            }
            QModelIndex new_node = createIndex(i - 1, y, row * gcol + y);
            nodes.insert(new_node, nodes.value(cur));
            link.insert(new_node,link.value(cur));
            nodes.remove(cur);
            link.remove(cur);
        }
    }

    endRemoveRows();
    return true;
}

void TreeItemModel::setIgnoreFilters(bool ignore)
{
    ignore_flag = ignore;
}

QModelIndex TreeItemModel::indexById(int id) const
{
    QModelIndexList keys = nodes.keys();
    for(int i = 0; i < keys.size(); i++)
    {
        if(keys.value(i).column()!=1)continue;
        if(nodes.value(keys.value(i)) == id)
            return keys.value(i);
    }
    return QModelIndex();
}

bool TreeItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    beginInsertRows(parent,row,row + count - 1);
    if(rowCount(parent) - 1 > row)
    {
        QModelIndex cur;
        for(int i = rowCount(parent) - 1 ; i >= row; i-- )
        {
            for(int y = 0; y < gcol; y++)
            {
                cur = index(i, y, parent);
                QModelIndex new_node = createIndex(i + count, y, gcol * row + y);
                nodes.insert(new_node, nodes.value(cur));
                link.insert(new_node, parent);
                nodes.insert(cur, QVariant());
            }
        }
    }
    else
    {
        for(int i = 0; i < count; i++)
        {
            for(int y = 0; y < gcol; y++)
            {
                QModelIndex cur = createIndex(row + i, y, gcol * row + y + qrand());
                nodes.insert(cur, QVariant());
                link.insert(cur,parent);
            }
        }
    }
    endInsertRows();
    return true;
}

bool TreeItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!nodes.contains(index) || role != Qt::EditRole)return false;
    nodes.insert(index,value);
    emit dataChanged(index,index);
    return true;
}

Qt::DropActions TreeItemModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool TreeItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    if(action == Qt::IgnoreAction)
        return false;

    if(!data->hasFormat("application/sql.tree.record"))
        return false;

    if(column > 0)
        return false;

    QString dt = data->data("application/sql.tree.record");
    if(dt.right(1) == ",") dt.chop(1);

    int prnt = this->data(index(parent.row(),1,parent.parent()),100).toInt();

    QString sql = QString("UPDATE categories SET parent_id=%1 WHERE id IN (%2)").arg(QString::number(prnt),dt);

    QSqlQuery query(sql);
    if(!query.exec())
    {
        qDebug()<<"TreeItemModel::dropMimeData(1)" + query.executedQuery() + " Error:" + query.lastError().text();
        return false;
    }

    QStringList rows = dt.split(",");
    for(int i = 0; i < rows.size(); ++i) //проходим по всем строкам модели, которые перемещаются
    {
        QModelIndex sourceidx = indexById(rows.value(i).toInt()); //узнаем индекс перемещаемой строки
        sourceidx = index(sourceidx.row(),0,sourceidx.parent());
        if(sourceidx.parent() == parent) return false; //если родительские узлы места назначения и переносимой строки совпадают, то ничего не делаем

        beginMoveRows(sourceidx.parent(),sourceidx.row(),sourceidx.row(),parent,rowCount(parent));
        insertRow(rowCount(parent),parent);
        for(int y = 0; y < columnCount(sourceidx.parent()); y++)
        {
            QModelIndex cur_src = index(sourceidx.row(),y,sourceidx.parent());
            QModelIndex cur = index(rowCount(parent)-1,y,parent);
            setData(cur,this->data(cur_src));
        }

        QModelIndexList children = link.keys(sourceidx);
        for(int y = 0; y < children.size(); y++)
            link.insert(children.value(y),index(rowCount(parent)-1,0,parent));
        removeRow(sourceidx.row(),sourceidx.parent());
        endMoveRows();
    }
    return true;
}

QMimeData* TreeItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimedata = new QMimeData();
    QByteArray encoded;

    for(int i = 0; i < indexes.size(); i++)
    {
        QModelIndex idx = index(indexes.value(i).row(),1,indexes.value(i).parent());
        QString dt = data(idx,Qt::DisplayRole).toString() + ",";
        encoded.append(dt);
    }

    mimedata->setData("application/sql.tree.record",encoded);

    return mimedata;
}

QStringList TreeItemModel::mimeTypes() const
{
    QStringList mimetypes;
    mimetypes << "application/sql.tree.record";
    return mimetypes;
}

int TreeItemModel::taskCount(const QModelIndex &index,  bool incomplete) const
{
    int cnt = 0;
    int id = nodes.value(this->index(index.row(),1,index.parent())).toInt();
    QString filter = incomplete ? QString(" AND tstatus <> %1").arg(QString::number((int)LInterface::FINISHED)) : "";
    QSqlQuery query;
    query.prepare("SELECT id FROM categories WHERE parent_id=:id");
    query.bindValue("id",id);

    if(!query.exec())
    {
        qDebug()<<"TreeItemModel::taskCount(1)" + query.executedQuery() + " Error:" + query.lastError().text();
        return 0;
    }

    while(query.next())
        cnt += taskCount(indexById(query.value(0).toInt()),incomplete);

    query.clear();
    query.prepare("SELECT COUNT(*) FROM tasks WHERE categoryid=:id" + filter);
    query.bindValue("id",id);

    if(!query.exec())
    {
        qDebug()<<"TreeItemModel::taskCount(2)" + query.executedQuery() + " Error:" + query.lastError().text();
        return 0;
    }

    query.next();
    cnt += query.value(0).toInt();

    return cnt;
}
