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
    grow = 0;
    qr = 0;
    hosts.insert(0,QModelIndex());
}

TreeItemModel::~TreeItemModel()
{

}

QVariant TreeItemModel::data(const QModelIndex &index, int role) const
{
    if(index == hosts.value(1))
    {
        switch(role)
        {
        case Qt::DisplayRole: return tr("All Downloads");
        default: return QVariant();
        }
    }
    if(index.parent() == hosts.value(1))
    {
        switch(role)
        {
        case Qt::DisplayRole: qr->seek(index.row()); return qr->value(1).toString();
        default: return QVariant();
        }
    }
    return QVariant();
}

int TreeItemModel::rowCount(const QModelIndex &parent) const
{
    if(parent == QModelIndex())return 1;
    if(hosts.key(parent, -1) < 0)return 0;
    int id_hosts = hosts.key(parent);
    return link.keys(id_hosts).size();
}

int TreeItemModel::columnCount(const QModelIndex &parent) const
{
    if(parent == QModelIndex())return 1;
    return 1;
}

QModelIndex TreeItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if(column > columnCount(parent) || row > rowCount(parent) || hosts.key(parent, -1) < 0)return QModelIndex();
    int id_parent = hosts.key(parent);
    return link.keys(id_parent).value(row);
}

QModelIndex TreeItemModel::parent(const QModelIndex &child) const
{
    if(!link.contains(child)) return QModelIndex();
    int id_parent = link.value(child);
    return hosts.value(id_parent);
}

bool TreeItemModel::updateModel(const QSqlDatabase &db)
{
    if(qr)delete(qr);
    qr = 0;

    qr = new QSqlQuery("SELECT * FROM categories",db);
    if(!qr->exec())
    {
        reset();
        qDebug()<<"FUCK!";
        return false;
    }

    hosts.insert(1,createIndex(0,0,1));
    link.insert(hosts.value(1),0);
    while(qr->next())
    {
        hosts.insert(hosts.size(),createIndex(grow,0,grow));
        link.insert(hosts.value(hosts.size()-1),1);
        ++grow;
    }
    reset();
    return true;
}

bool TreeItemModel::hasChildren(const QModelIndex &parent) const
{
    int id_host = hosts.key(parent, -1);
    if(id_host < 0)return false;
    if(link.keys(id_host).size() > 0)return true;
    return false;
}

bool TreeItemModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    return true;
}
