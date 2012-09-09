/*
Project: REXLoader (Downloader), Source file: pluginlistmodel.cpp
Copyright (C) 2011-2012  Sarvaritdinov R.

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

#include "pluginlistmodel.h"

PluginListModel::PluginListModel(QObject *parent) :
    QAbstractListModel(parent)
{
    plugfiles = 0;
    pluglist = 0;
    plugproto = 0;
}

bool PluginListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    return false;
}

QModelIndex PluginListModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex idx = createIndex(row,column,row);
    return idx;
}

QVariant PluginListModel::data(const QModelIndex &index, int role) const
{
    if(!plugproto || !plugfiles || !pluglist)
        return QVariant();

    switch(role)
    {
    case Qt::DisplayRole:
    {
        QStringList protolist = plugproto->keys();
        QStringList pluginfo = pluglist->value(plugproto->value(protolist.value(index.row())))->pluginInfo();
        QString out = tr("Протокол %1 Плагин %2 версия %3\nАвтор: %4, (лицензия %5)").arg(protolist.value(index.row()).toUpper());
        return out;
    }

    default: return QVariant();
    }
}

int PluginListModel::rowCount(const QModelIndex &parent) const
{
    int sz = (plugproto ? plugproto->size(): 0);
    return sz;
}

int PluginListModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

void PluginListModel::setSorces(QHash<int, QString> *plugdirs, QHash<int, LoaderInterface *> *plglst, QHash<QString, int> *plgproto)
{
    plugfiles = plugdirs;
    pluglist = plglst;
    plugproto = plgproto;
}

void PluginListModel::addPluginCategory(const QString &name)
{
    if(categories.contains(name))
        return;

    categories.insert(name,categories.size()+1);
}
