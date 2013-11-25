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
#include <QtSql/QtSql>
#include <QAction>
#include <QMenu>
#include <QTableView>
#include <QItemSelectionModel>
#include <QSortFilterProxyModel>

#include "../plugins/LoaderInterface.h"
#include "../plugins/NotifInterface.h"
#include "../plugins/FileInterface.h"
#include "titemmodel.h"
#include "logtreemodel.h"
#include "pluginlistmodel.h"
#include "site_manager/sitemanager.h"

class PluginOperator : public QObject
{
    Q_OBJECT
public:
    explicit PluginOperator(QObject *parent = 0);
    void setPluglist(QHash<int,LoaderInterface*> *list);

public slots:
    void startDownload(int id_task);
    void stopDownload(int id_task);
    void setAuthorizationData(int id_task, const QString &auth);

private:
    QHash<int,LoaderInterface*> *pluglist; //хэш ссылок на плагины
};

class UpdaterOperator : public QObject
{
    Q_OBJECT
public:
    explicit UpdaterOperator(QObject *parent = 0);
    ~UpdaterOperator();
    bool openDatabase(const QString &dbfile);

public slots:
    void execQuery(const QString &query);
};

class PluginManager : public QThread
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = 0);
    ~PluginManager();

    void isFirstRun(bool frst = true);
    void setPlugDir(const QStringList &dir);
    void setPlugLists(QHash<int,QString> *files, QHash<int,LoaderInterface*> *list, QHash<QString,int> *proto, QHash<int,int> *tsklist);
    void setDefaultSettings(const int &tasks, const int &threads, const qint64 &speed, const int &att_interval);
    void setDatabaseFile(const QString &dbfile);
    void setSiteManager(SiteManager *mgr);
    void loadLocale(const QLocale &locale);
    void restorePluginsState(const QByteArray &stat);
    void setPluginListModel(PluginListModel *mdl);
    void setTaskTable(QTableView *tbl);
    QPair<NotifInterface*,int>* getNotifPlugin();
    QHash<QString,FileInterface*> *getFilePlugin();
    QHash<int,QWidget*> *getPlugWidgets();
    QString pluginInfo(const LoaderInterface *ldr,const QString &call) const;
    QByteArray pluginsState() const;
    QMenu* filePluginMenu() const;

signals:
    void pluginStatus(bool stat);
    void startTask(int id_task);
    void stopTask(int id_task);
    void needExecQuery(const QString &query);
    void messageAvailable(int id_task, int id_sect, int ms_type, const QString &title, const QString &more);
    void notifActionInvoked(const QString &act);
    void needLoadOtherPlugin(const QString &filepath);
    void needLoadNotifPlugin(int id);
    void needCreatePlugWidget(int plug_id);
    void authData(int id_task, const QString &auth);

public slots:
    void startDownload(int id_task);
    void stopDownload(int id_tsk);
    void setAuthorizationData(int id_task, const QString &auth);
    void exeQuery(const QString &query);
    void appendLog(int id_task, int id_sect, int ms_type, const QString &title, const QString &more);
    void notify(const QString &title, const QString &msg, int timeout = 10, const QStringList &acts = QStringList(), int type = NotifInterface::INFO, QImage* img=0);
    void notifActRecv(unsigned int, const QString &act);
    void actionAnalizer();
    void updateFilePluginMenu();
    void unloadOtherPlugin(const QString &plgid);
    void needAuthorization(int id_task, const QUrl &url);

protected:
    void run();

protected slots:
    void loadOtherPlugin(const QString &filepath);
    void loadNotifPlugin(int id);
    void createPlugWidget(int plug_id);

private:
    const QStringList *pluginDirs; //список с директориями, в которых могут быть плагины
    QHash<int,QString> *plugfiles; //хэш путей к файлам плагинов
    QHash<int,LoaderInterface*> *pluglist; //хэш ссылок на плагины
    QHash<QString,int> *plugproto; //хэш дескрипторов плагинов с соответствующими протоколами
    QHash<int,int> *tasklist;
    QHash<QString,QTranslator*> translators;
    QHash<int,QStringList> notifplugins; //данные о доступных плагинах уведомлений
    QPair<NotifInterface*,int> notifplugin; //активный плагин уведомления
    QHash<QString,QStringList> fileplugins; //данные о доступных файловых плагинах
    QHash<QString,FileInterface*> fileplugin; //ссылки на активные файловые плагины

    const int *max_tasks; //максимальное количество одновременных закачек
    const int *max_threads; //максимальное кол-во потоков при скачивании
    const qint64 *down_speed; //максимальная скорость скачивания
    const int *attempt_interval; //интервал между посылами запроса потоков
    QImage *appIcon;

    LogTreeModel *logmodel;

    UpdaterOperator *updOper;
    QString db;
    bool first_run;
    QHash<QAction*,QPair<FileInterface*,int> > act_table; //хэш связей QAction с файл-плагинами
    QMenu *menu;
    QTableView *tab_view;
    QHash<int, QWidget*> plug_widgets;
    QLocale _locale;
    SiteManager *site_mgr;
};

#endif // PLUGINMANAGER_H
