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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QThread>
#include <QDir>
#include <QHash>
#include <QStringList>
#include <QPluginLoader>
#include <QDebug>
#include <QApplication>
#include <QTranslator>

#include "../Httploader/LoaderInterface.h"

class PluginOperator : public QObject
{
    Q_OBJECT
public:
    explicit PluginOperator(QObject *parent = 0);
    void setPluglist(QHash<int,LoaderInterface*> *list);

public slots:
    void startDownload(int id_task);
    void stopDownload(int id_task);

private:
    QHash<int,LoaderInterface*> *pluglist; //хэш ссылок на плагины
};

class PluginManager : public QThread
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = 0);

    void setPlugDir(const QStringList &dir);
    void setPlugLists(QHash<int,QString> *files, QHash<int,LoaderInterface*> *list, QHash<QString,int> *proto);
    void setDefaultSettings(const int &tasks, const int &threads, const qint64 &speed);
    void loadLocale(const QLocale &locale);
    void restorePluginsState(const QByteArray &stat);
    QByteArray pluginsState() const;

signals:
    void pluginStatus(bool stat);
    void startTask(int id_task);
    void stopTask(int id_task);

public slots:
    void startDownload(int id_task);
    void stopDownload(int id_tsk);

protected:
    void run();

private:
    const QStringList *pluginDirs; //список с директориями, в которых могут быть плагины
    QHash<int,QString> *plugfiles; //хэш путей к файлам плагинов
    QHash<int,LoaderInterface*> *pluglist; //хэш ссылок на плагины
    QHash<QString,int> *plugproto; //хэш дескрипторов плагинов с соответствующими протоколами
    QHash<int,QTranslator*> translators;

    const int *max_tasks; //максимальное количество одновременных закачек
    const int *max_threads; //максимальное кол-во потоков при скачивании
    const qint64 *down_speed;
};

#endif // PLUGINMANAGER_H
