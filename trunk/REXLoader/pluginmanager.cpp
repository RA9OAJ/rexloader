/*
Project: REXLoader (Downloader), Source file: pluginmanager.cpp
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

#include "pluginmanager.h"

PluginManager::PluginManager(QObject *parent) :
    QThread(parent)
{
    pluginDirs = 0;
    plugfiles = 0;
    plugproto = 0;
    max_tasks = 0;
    max_threads = 0;
    tasklist = 0;
    attempt_interval = 0;
    notifplugin.first = 0;
    notifplugin.second = 0;
    appIcon = new QImage(":/appimages/trayicon.png");
    *appIcon = appIcon->scaledToWidth(64,Qt::SmoothTransformation).rgbSwapped();
    connect(this,SIGNAL(needLoadOtherPlugin(QString)),this,SLOT(loadOtherPlugin(QString)),Qt::QueuedConnection);
    connect(this,SIGNAL(needLoadNotifPlugin(int)),this,SLOT(loadNotifPlugin(int)),Qt::QueuedConnection);
    first_run = false;
    updOper = 0;
    menu = new QMenu();
}

PluginManager::~PluginManager()
{
    delete appIcon;
}

void PluginManager::setPlugDir(const QStringList &dir)
{
    pluginDirs = &dir;
}

void PluginManager::setPlugLists(QHash<int, QString> *files, QHash<int, LoaderInterface *> *list, QHash<QString, int> *proto, QHash<int, int> *tsklist)
{
    plugfiles = files;
    pluglist = list;
    plugproto = proto;
    tasklist = tsklist;
}

void PluginManager::run()
{
    if(!plugfiles || !pluglist || !plugproto || !pluginDirs){emit pluginStatus(false); return;}
    PluginOperator *oper = new PluginOperator();
    connect(this,SIGNAL(startTask(int)),oper,SLOT(startDownload(int)),Qt::QueuedConnection);
    connect(this,SIGNAL(stopTask(int)),oper,SLOT(stopDownload(int)),Qt::QueuedConnection);
    oper->setPluglist(pluglist);

    for(int i=0; i<pluginDirs->size(); i++)
    {
        QDir dir(pluginDirs->value(i));
        QStringList plg = dir.entryList(QDir::Files);

        for(int y=0; y<plg.size(); y++)
        {
            QPluginLoader plug(pluginDirs->value(i)+"/"+plg.value(y));
            if(!plug.load())continue;
            QObject *pobject = plug.instance();
            LoaderInterface *ldr = qobject_cast<LoaderInterface*>(pobject);
            if(!ldr)
            {
                NotifInterface *nldr = qobject_cast<NotifInterface*>(pobject);
                if(!nldr)
                {
                    FileInterface *fplg = qobject_cast<FileInterface*>(pobject);
                    if(fplg)
                        fileplugins.insert(pluginDirs->value(i)+"/"+plg.value(y),fplg->pluginInfo());

                    plug.unload();
                    //emit needLoadOtherPlugin(pluginDirs->value(i)+"/"+plg.value(y));
                    continue;
                }

                QStringList pluginfo = nldr->pluginInfo();
                pluginfo << QString("Filepath: ")+pluginDirs->value(i)+"/"+plg.value(y);
                notifplugins.insert(notifplugins.size()+1,pluginfo);
                if(!notifplugin.first && first_run)
                {
                    plug.unload();
                    emit needLoadNotifPlugin(notifplugins.size());
                    /*notifplugin.first = nldr;
                    connect(nldr,SIGNAL(notifyActionData(uint,QString)),this,SLOT(notifActRecv(uint,QString)));
                    notifplugin.second = notifplugins.size();*/
                }
                else
                {
                    delete nldr;
                    plug.unload();
                }

                continue;
            }
            emit messageAvailable(-1,0,LInterface::MT_INFO,tr("Плагин %1 версия %2-%3 ('%4') загружен.").arg(pluginInfo(ldr,"Plugin"),pluginInfo(ldr,"Version"),pluginInfo(ldr,"Build date"),pluginDirs->value(i)+"/"+plg.value(y)),QString());
            pluglist->insert(pluglist->size()+1,ldr);
            plugfiles->insert(pluglist->size(),pluginDirs->value(i)+"/"+plg.value(y));
            connect(ldr,SIGNAL(messageAvailable(int,int,int,QString,QString)),this,SLOT(appendLog(int,int,int,QString,QString)),Qt::QueuedConnection);

            QStringList protocols = ldr->protocols();
            for(int x=0; x<protocols.size(); x++)
            {
                if(plugproto->value(protocols.value(x)))continue;
                plugproto->insert(protocols.value(x),pluglist->size());
                ldr->setMaxSectionsOnTask(*max_threads);
                ldr->setDownSpeed(*down_speed*1024/8);
                ldr->setAttemptInterval(*attempt_interval);
            }
        }
    }

    bool stat = false;
    if(pluglist->size())stat = true;
    if(stat)
    {
        UpdaterOperator *op = new UpdaterOperator();

        op->openDatabase(db);
        connect(this,SIGNAL(needExecQuery(QString)),op,SLOT(execQuery(QString)),Qt::QueuedConnection);
    }

    emit pluginStatus(stat);
    exec();
}

void PluginManager::updateFilePluginMenu()
{
    menu->clear();
    menu->addActions(act_table.keys());
}

void PluginManager::loadOtherPlugin(const QString &filepath)
{
    if(fileplugin.contains(filepath))
        return;

    QPluginLoader pldr(filepath);
    if(!pldr.load())
        return;

    FileInterface *plg = qobject_cast<FileInterface*>(pldr.instance());
    if(!plg)
    {
        pldr.unload();
        return;
    }

    QStringList pluginfo = plg->pluginInfo();
    QString plgid = filepath;
    if(!fileplugins.contains(plgid))
        fileplugins.insert(plgid,pluginfo);
    fileplugin.insert(plgid,plg);

    QList<DataAction> actions = plg->getActionList();
    foreach(DataAction dact, actions)
    {
        QAction *act = new QAction(dact.act_title,this);
        act->setIcon(dact.act_icon);
        QPair<FileInterface*,int> ref;
        ref.first = plg;
        ref.second = dact.act_id;
        act_table.insert(act,ref);
        connect(act,SIGNAL(triggered()),this,SLOT(actionAnalizer()));
    }

    updateFilePluginMenu();
}

void PluginManager::loadNotifPlugin(int id)
{
    if(!notifplugins.contains(id))
        return;

    if(notifplugin.first)
        delete notifplugin.first;

    PluginInfo plginfo(notifplugins.value(id));
    QPluginLoader pldr(plginfo.filepath);
    if(!pldr.load())
        return;

    NotifInterface *plg = qobject_cast<NotifInterface*>(pldr.instance());
    if(!plg)
    {
        pldr.unload();
        return;
    }

    notifplugin.first = plg;
    notifplugin.second = id;
    connect(plg,SIGNAL(notifyActionData(uint,QString)),this,SLOT(notifActRecv(uint,QString)));
}

void PluginManager::setDefaultSettings(const int &tasks, const int &threads, const qint64 &speed, const int &att_interval)
{
    max_tasks = &tasks;
    max_threads = &threads;
    down_speed = &speed;
    attempt_interval = &att_interval;
}

void PluginManager::startDownload(int id_task)
{
    emit startTask(id_task);
}

void PluginManager::stopDownload(int id_tsk)
{
    emit stopTask(id_tsk);
}

void PluginManager::exeQuery(const QString &query)
{
    emit needExecQuery(query);
}

void PluginManager::appendLog(int id_task, int id_sect, int ms_type, const QString &title, const QString &more)
{
    if(!tasklist) return;
    LoaderInterface *ldr = qobject_cast<LoaderInterface*>(sender());
    if(!ldr) return;
    if(id_task > 0)
    {
        int new_id = pluglist->key(ldr) * 100 + id_task;
        new_id = tasklist->key(new_id);
        emit messageAvailable(new_id,id_sect,ms_type,title,more);
    }
    else emit messageAvailable(id_task,id_sect,ms_type,title,more);
}

void PluginManager::notify(const QString &title, const QString &msg, int timeout, const QStringList &acts, int type, QImage *img)
{
    if(!notifplugin.first)
        return;

    QImage *image = appIcon;
    if(img)
    {
        image = img;
        *image = image->scaledToWidth(64,Qt::SmoothTransformation).rgbSwapped();
    }

    notifplugin.first->notify(qApp->applicationName(),title,msg,timeout,type,acts,image);
}

void PluginManager::notifActRecv(unsigned int, const QString &act)
{
    emit notifActionInvoked(act);
}

void PluginManager::actionAnalizer()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if(!act)
        return;

    if(act_table.contains(act))
        act_table.value(act).first->runAction(act_table.value(act).second);
}

void PluginManager::setDatabaseFile(const QString &dbfile)
{
    db = dbfile;
}

//---------------------PluginOperator----------------------

PluginOperator::PluginOperator(QObject *parent) :
    QObject(parent)
{
    pluglist = 0;
}

void PluginOperator::setPluglist(QHash<int, LoaderInterface *> *list)
{
    pluglist = list;
}

void PluginManager::isFirstRun(bool frst)
{
    first_run = frst;
}

void PluginOperator::startDownload(int id_task)
{
    if(!pluglist)return;
    int task = id_task%100;
    if(!task)return;
    int plug = id_task/100;
    pluglist->value(plug)->startDownload(task);
}

void PluginOperator::stopDownload(int id_task)
{
    if(!pluglist)return;
    int task = id_task%100;
    if(!task)return;
    int plug = id_task/100;
    pluglist->value(plug)->stopDownload(task);
}

void PluginManager::loadLocale(const QLocale &locale)
{
    QList<int> keys;

    if(!translators.isEmpty())
    {
        keys = translators.keys();
        for(int i = 0; i < keys.size(); ++i)
        {
            qApp->removeTranslator(translators.value(keys.value(i)));
            translators.value(keys.value(i))->deleteLater();
            translators.remove(keys.value(i));
        }
        keys.clear();
    }

    keys = pluglist->keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        QTranslator *translator = pluglist->value(keys.value(i))->getTranslator(locale);
        if(!translator) continue;
        translator->moveToThread(thread());
        qApp->installTranslator(translator);
        translators.insert(keys.value(i),translator);
    }
}

void PluginManager::restorePluginsState(const QByteArray &stat)
{
    if(!plugfiles || !pluglist || !plugproto || !pluginDirs) return;
    if(!plugfiles->size() || !pluglist->size()) return;

    QHash<QString,int> last_state = *plugproto;
    plugproto->clear();
    if(!fileplugin.isEmpty())
    {
        QStringList plgs = fileplugin.keys();
        foreach(QString plg,plgs)
            if(fileplugin.value(plg))
                delete fileplugin.value(plg);

        fileplugin.clear();
    }

    QDataStream in(stat);

    int len;
    in >> len; //считываем размер первой строки с протоколом

    while(len > 0)
    {
        QByteArray inbuf;
        inbuf.resize(len);
        in.readRawData(inbuf.data(),len); //считываем строку с названием протокола
        QString proto = inbuf;
        if(proto == "")break; //если не удалось считать, то выходим
        if(proto == "NOTIF")
        {
            in >> len; //размер строки пути до файла NOTIF плагина
            if(!len)
            {
                notifplugin.first = 0;
                notifplugin.second = 0;
                break;
            }
            inbuf.clear();
            inbuf.resize(len);
            in.readRawData(inbuf.data(),len); //считываем путь до NOTIF плагина

            QString filepath = inbuf;
            for(int i = 1; i <= notifplugins.size(); ++i)
                if(notifplugins.value(i).last() == QString("Filepath: ")+filepath && notifplugin.second != i)
                {
                    loadNotifPlugin(i);
                    break;
                }

            in >> len;
            continue;
        }
        if(proto == "FILE")
        {
            in >> len;
            if(!len) break;

            inbuf.clear();
            inbuf.resize(len);
            in.readRawData(inbuf.data(),len);

            QString filepath = inbuf;
            QPluginLoader ldr(inbuf);
            if(ldr.load())
                loadOtherPlugin(filepath);

            in >> len;
            continue;
        }

        in >> len; //считываем размер строки пути до файла плагина
        if(!len) break;
        if(len == -1)
        {
            in >> len;
            plugproto->insert(proto,0);
            continue;
        }

        inbuf.clear();
        inbuf.resize(len);
        in.readRawData(inbuf.data(),len); //считываем строку пути до файла плагина
        QString filepath = inbuf;
        if(filepath == "") //если не удалось считать
            break;

        in >> len;
        int id = plugfiles->key(filepath);
        if(!pluglist->contains(id) || !pluglist->value(id)) continue;
        plugproto->insert(proto,id);
    }
    if(!plugproto->size()) *plugproto = last_state;
}

void PluginManager::setPluginListModel(PluginListModel *mdl)
{
    if(mdl)
    {
        mdl->setOtherPluginSources(&notifplugins, &notifplugin, &fileplugins, &fileplugin);
        connect(mdl,SIGNAL(needLoadNotifPlugin(int)),this,SLOT(loadNotifPlugin(int)));
        connect(mdl,SIGNAL(needLoadOtherPlugin(QString)),this,SLOT(loadOtherPlugin(QString)));
    }
}

QPair<NotifInterface *, int> *PluginManager::getNotifPlugin()
{
    return &notifplugin;
}

QHash<QString, FileInterface *> *PluginManager::getFilePlugin()
{
    return &fileplugin;
}

QString PluginManager::pluginInfo(const LoaderInterface *ldr, const QString &call) const
{
    QMap<QString,QString> calls;
    QStringList lst = ldr->pluginInfo();
    if(lst.isEmpty()) return QString();

    for(int i = 0; i < lst.size(); ++i)
    {
        QStringList data = lst.value(i).split(": ");
        calls.insert(data.value(0).toLower(),data.value(1));
    }
    return calls.value(call.toLower(),QString());
}

QByteArray PluginManager::pluginsState() const
{
    if(!plugfiles || !pluglist || !plugproto || !pluginDirs) return QByteArray();
    if(!plugfiles->size() || !plugproto->size()) return QByteArray();

    QByteArray out;
    QDataStream stat(&out,QIODevice::WriteOnly);

    foreach(QString proto, plugproto->keys())
    {
        if(!proto.toAscii().size()) continue;
        stat << proto.toAscii().size(); //размерность строки с названием протокола
        stat.writeRawData(proto.toAscii().data(),proto.toAscii().size()); //строка с названием протокола
        int id = plugproto->value(proto); //id плагина
        QString filepath = plugfiles->value(id,QString()); //путь до плагина

        stat << ((filepath.toAscii().size() > 0) ? filepath.toAscii().size() : -1); //размер строки пути файла плагина
        if(filepath == "") continue;

        stat.writeRawData(filepath.toAscii().data(),filepath.toAscii().size()); //путь до файла плагина
    }

    if(notifplugin.second)
    {
        QString notif = "NOTIF";
        stat << notif.toAscii().size();
        stat.writeRawData(notif.toAscii().data(),notif.toAscii().size());

        QString filepath = notifplugins.value(notifplugin.second).last();
        filepath = filepath.replace("Filepath: ","");
        stat << filepath.toAscii().size();
        stat.writeRawData(filepath.toAscii().data(),filepath.toAscii().size());
    }

    if(!fileplugin.isEmpty())
    {
        QStringList plgs = fileplugin.keys();
        QString filepath;
        foreach(filepath,plgs)
        {
            QString file = "FILE";
            stat << file.toAscii().size();
            stat.writeRawData(file.toAscii().data(),file.toAscii().size());
            stat << filepath.toAscii().size();
            stat.writeRawData(filepath.toAscii().data(),filepath.toAscii().size());
        }
    }

    stat << (int) 0;

    return out;
}

QMenu *PluginManager::filePluginMenu() const
{
    return menu;
}

//-----------------------UpdaterOperator-----------------

UpdaterOperator::UpdaterOperator(QObject *parent) :
    QObject::QObject(parent)
{
}

bool UpdaterOperator::openDatabase(const QString &dbfile)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE","sqliteConnection");
    db.setDatabaseName(dbfile);

    return db.open();
}

void UpdaterOperator::execQuery(const QString &query)
{
    QSqlQuery qr;
    qr.prepare(query);
    if(!qr.exec())
    {
        qDebug()<<"SQL Error:" + qr.executedQuery() + " Error: " + qr.lastError().text();
    }
}

UpdaterOperator::~UpdaterOperator()
{
}
