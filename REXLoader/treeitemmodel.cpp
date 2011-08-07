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
}

TreeItemModel::~TreeItemModel()
{

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
        qDebug()<<"FUCK!";
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
    return true;
}

bool TreeItemModel::hasChildren(const QModelIndex &parent) const
{
    if(link.keys(parent).size())return true;
    return false;
}

bool TreeItemModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    return true;
}
