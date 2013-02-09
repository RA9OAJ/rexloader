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
    Q_UNUSED(data)
    Q_UNUSED(action)
    Q_UNUSED(row)
    Q_UNUSED(column)
    Q_UNUSED(parent)
    return false;
}

QModelIndex PluginListModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    QModelIndex idx = createIndex(row,column,row);
    return idx;
}

QVariant PluginListModel::data(const QModelIndex &index, int role) const
{
    if(!plugproto || !plugfiles || !pluglist)
        return QVariant();

    if(index.row() < plugproto->size())
    {
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
        case PluginListModel::PlugType:
        {
            return QString("Loader");
        }

        default: return QVariant();
        }
    }
    else if(index.row() >= plugproto->size() + notifplugins->size())
    {
        QStringList plgs = fileplugins->keys();
        QString plgpath = plgs.value(index.row() - plugproto->size() - notifplugins->size());
        PluginInfo pluginfo(fileplugins->value(plgpath));

        switch(role)
        {
        case Qt::DisplayRole:
        {
            QStringList out;
            out << pluginfo.name << pluginfo.version << pluginfo.authors << pluginfo.license;
            return out;
        }
        case PluginListModel::PlugName:
        {
            return pluginfo.name;
        }
        case PluginListModel::PlugState:
        {
            if(fileplugin->contains(plgpath))
                return true;
            return false;
        }
        case PluginListModel::PlugType:
            return QString("File");
        case PluginListModel::PlugId:
        {
            return plgpath;
        }

        default: return QVariant();
        }
    }
    else
    {
        QList<int> plgs = notifplugins->keys();
        PluginInfo pluginfo(notifplugins->value(plgs.value(index.row() - plugproto->size())));

        switch(role)
        {
        case Qt::DisplayRole:
        {
            QStringList out;
            out<<pluginfo.name<<pluginfo.version<<pluginfo.authors<<pluginfo.license;
            return out;
        }
        case PluginListModel::PlugName:
        {
            return pluginfo.name;
        }
        case PluginListModel::PlugId:
        {
            return plgs.value(index.row() - plugproto->size());
        }
        case PluginListModel::PlugType:
            return QString("Notify");

        default: return QVariant();
        }
    }

    return QVariant();
}

int PluginListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    int sz = (plugproto ? plugproto->size(): 0);
    sz += (notifplugins ? notifplugins->size(): 0);
    sz += (fileplugins ? fileplugins->size(): 0);
    return sz;
}

int PluginListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

bool PluginListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(role)
    if(index.row() < plugproto->size())
        plugproto->insert(index.data(ProtocolName).toString().toLower(),value.toInt());
    else if(index.row() >= plugproto->size() + notifplugins->size())
    {
        QString plgid = index.data(PlugId).toString();
        if(!fileplugin->contains(plgid) && value.toBool())
        {
            QPluginLoader ldr;
            ldr.setFileName(plgid);
            if(!ldr.load())
                return true;

            FileInterface *plg = qobject_cast<FileInterface*>(ldr.instance());
            if(plg) fileplugin->insert(plgid,plg);
        }
        else if(!fileplugin->contains(plgid))
        {
            QString plgid = index.data(PlugId).toString();
            if(fileplugin->contains(plgid) && fileplugin->value(plgid))
            {
                delete fileplugin->value(plgid);
                fileplugin->remove(plgid);
            }
        }
    }
    else
    {

    }

    return true;
}

void PluginListModel::setSorces(QHash<int, QString> *plugdirs, QHash<int, LoaderInterface *> *plglst, QHash<QString, int> *plgproto)
{
    plugfiles = plugdirs;
    pluglist = plglst;
    plugproto = plgproto;
}

void PluginListModel::setOtherPluginSources(QHash<int, QStringList> *notifplgs, QPair<NotifInterface*,int> *notifplg, QHash<QString, QStringList> *flplgs, QHash<QString, FileInterface *> *flplg)
{
    notifplugins = notifplgs;
    notifplugin = notifplg;
    fileplugins = flplgs;
    fileplugin = flplg;
}

QList<QPair<QString, int> > PluginListModel::pluginsList(const QModelIndex &index)
{
    QList<QPair<QString, int> > lst;

    if(index.data(PluginListModel::PlugType).toString() == "Loader")
    {
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
    }
    else if(index.data(PluginListModel::PlugType).toString() == "File")
        lst << QPair<QString,int>(tr("включено"),1);
    else
    {
        QList<int> keys = notifplugins->keys();
        int key;

        foreach(key,keys)
        {
            PluginInfo plginfo(notifplugins->value(key));
            lst << QPair<QString, int>(plginfo.name, key);
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
