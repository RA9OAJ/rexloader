/*
<one line to give the program's name and a brief idea of what it does.>
Copyright (C) <year>  <name of author>

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
            if(nodes.value(index).toString() == "#downloads")return tr("All Downloads");
            else if(nodes.value(index).toString() == "#archives")return tr("Archives");
            else if(nodes.value(index).toString() == "#apps") return tr("Applications");
            else if(nodes.value(index).toString() == "#audio") return tr("Music");
            else if(nodes.value(index).toString() == "#video") return tr("Video");
            else if(nodes.value(index).toString() == "#other") return tr("Other");
            return nodes.value(index).toString();
        }
        return nodes.value(index);

    case Qt::FontRole:
        if(!index.column()) return font;
        return QFont();

    case 100: return nodes.value(index);

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
    addFiltersSubtree();
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
    filters << tr("Filters")<< -1 << 0 << 0;
    filters << tr("Downloading")<< -2 << -1 << 3;
    filters << tr("Waiting") << -3 << -1 << -100;
    filters << tr("Stopped") << -4 << -1 << 0;
    filters << tr("Completed") << -5 << -1 << 5;
    filters << tr("With an error") << -6 << -1 << -2;

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
    addFiltersSubtree();
    return true;
}

void TreeItemModel::updateRow(int row, const QModelIndex &parent)
{

}

void TreeItemModel::insertRow(int row, const QModelIndex &parent)
{

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
    if(count > rowCount(parent))count = -1;
    QModelIndex cur = QModelIndex();

    beginRemoveRows(parent, row, row + count - 1);
    for(int i = row; i < count; ++i)
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
    endRemoveRows();
    return true;
}
