/*
Project: REXLoader (Downloader), Source file: pluginlistmodel.h
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

#ifndef PLUGINLISTMODEL_H
#define PLUGINLISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>
#include "../plugins/LoaderInterface.h"

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
};

class PluginListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum DataType{
        PlugName = 100,
        PlugId = 101,
        ProtocolName = 102
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
    QList<QPair<QString, int> > pluginsList(const QModelIndex &index);
signals:
    
public slots:

private:
    QHash<QString, int> categories;
    QHash<int,QString> *plugfiles; //хэш путей к файлам плагинов
    QHash<int,LoaderInterface*> *pluglist; //хэш ссылок на плагины
    QHash<QString,int> *plugproto; //хэш дескрипторов плагинов с соответствующими протоколами
};

#endif // PLUGINLISTMODEL_H
