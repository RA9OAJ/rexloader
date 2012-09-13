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
        QStringList out;
        QStringList protolist = plugproto->keys();
        out << protolist.value(index.row()).toUpper();

        if(plugproto->value(protolist.value(index.row())))
        {
            PluginInfo pluginfo(pluglist->value(plugproto->value(protolist.value(index.row())))->pluginInfo());
            out << pluginfo.name << pluginfo.version << pluginfo.authors << pluginfo.license;
        }
        else out << tr("отключено") << "" << "-" << "-" << "-";
        return out;
    }
    case PluginListModel::PlugName:
    {
        QStringList protolist = plugproto->keys();

        if(plugproto->value(protolist.value(index.row())))
        {
            PluginInfo pluginfo(pluglist->value(plugproto->value(protolist.value(index.row())))->pluginInfo());
            return pluginfo.name;
        }
        else return QVariant();
    }
    case PluginListModel::ProtocolName:
    {
        QStringList protolist = plugproto->keys();
        return protolist.value(index.row());
    }
    case PluginListModel::PlugId:
    {
        QStringList protolist = plugproto->keys();
        return plugproto->value(protolist.value(index.row()));
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

bool PluginListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    plugproto->insert(index.data(ProtocolName).toString().toLower(),value.toInt());
    return true;
}

void PluginListModel::setSorces(QHash<int, QString> *plugdirs, QHash<int, LoaderInterface *> *plglst, QHash<QString, int> *plgproto)
{
    plugfiles = plugdirs;
    pluglist = plglst;
    plugproto = plgproto;
}

QList<QPair<QString, int> > PluginListModel::pluginsList(const QModelIndex &index)
{
    QList<QPair<QString, int> > lst;
    QList<int> keys = pluglist->keys();
    int key;

    foreach(key,keys)
    {
        LoaderInterface *plg = pluglist->value(key);
        if(plg->protocols().contains(index.data(PluginListModel::ProtocolName).toString(),Qt::CaseInsensitive))
        {
            PluginInfo plginfo(plg->pluginInfo());
            lst << QPair<QString,int>(plginfo.name,key);
        }
    }

    return lst;
}

void PluginListModel::addPluginCategory(const QString &name)
{
    if(categories.contains(name))
        return;

    categories.insert(name,categories.size()+1);
}

PluginInfo::PluginInfo(const QStringList &params)
{
    QString cur;
    foreach(cur,params)
    {
        QStringList pair = cur.split(": ");
        if(pair.size() < 2)
            continue;

        if(pair.value(0).toLower() == "plugin")
            name = pair.value(1);
        else if(pair.value(0).toLower() == "authors")
            authors = pair.value(1).replace(QRegExp("[\r\n]+"),", ");
        else if(pair.value(0).toLower() == "place")
            place = pair.value(1);
        else if(pair.value(0).toLower() == "build date")
            builddate = pair.value(1);
        else if(pair.value(0).toLower() == "version")
            version = pair.value(1);
        else if(pair.value(0).toLower() == "lic")
            license = pair.value(1);
        else if(pair.value(0).toLower() == "description")
            description = pair.value(1);
    }
}

PluginInfo::~PluginInfo()
{
}
