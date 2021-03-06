/*
Copyright (C) 2012-2013  Sarvaritdinov R.

This file is part of REXLoader.

REXLoader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

REXLoader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLUGINLISTMODEL_H
#define PLUGINLISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include <QPluginLoader>
#include "../plugins/LoaderInterface.h"
#include "../plugins/NotifInterface.h"
#include "../plugins/FileInterface.h"

class PluginInfo
{
public:
    PluginInfo(const QStringList &params);
    ~PluginInfo();

    QString name;
    QString authors;
    QString place;
    QString builddate;
    QString version;
    QString license;
    QString description;
    QString filepath;
};

class PluginListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum DataType{
        PlugName = 100,
        PlugId = 101,
        ProtocolName = 102,
        PlugType = 103,
        PlugState = 104
    };

    explicit PluginListModel(QObject *parent = 0);
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex & parent = QModelIndex());
    virtual QModelIndex	index (int row, int column = 0, const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::UserRole);
    void addPluginCategory(const QString &name);
    void setSorces(QHash<int,QString> *plugdirs, QHash<int,LoaderInterface*> *plglst, QHash<QString,int> *plgproto);
    void setOtherPluginSources(QHash<int,QStringList> *notifplgs, QPair<NotifInterface*,int> *notifplg, QHash<QString,QStringList> *flplgs, QHash<QString,FileInterface*> *flplg);
    QList<QPair<QString, int> > pluginsList(const QModelIndex &index);

signals:
    void needLoadOtherPlugin(const QString &filepath);
    void needLoadNotifPlugin(int id);
    void needUnloadOtherPlugin(const QString &plgid);
    void needUpdatePlugMenu();
    
public slots:

private:
    QHash<QString, int> categories;
    QHash<int,QString> *plugfiles; //хэш путей к файлам плагинов
    QHash<int,LoaderInterface*> *pluglist; //хэш ссылок на плагины
    QHash<QString,int> *plugproto; //хэш дескрипторов плагинов с соответствующими протоколами
    QHash<int,QStringList> *notifplugins; //данные о доступных плагинах уведомлений
    QPair<NotifInterface*,int> *notifplugin; //активный плагин уведомления
    QHash<QString,QStringList> *fileplugins; //данные о доступных файловых плагинах
    QHash<QString,FileInterface*> *fileplugin; //ссылки на активные файловые плагины
};

#endif // PLUGINLISTMODEL_H
