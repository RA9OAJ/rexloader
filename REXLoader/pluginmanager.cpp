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
    logmodel = 0;

    updOper = 0;
}

PluginManager::~PluginManager()
{
}

void PluginManager::setPlugDir(const QStringList &dir)
{
    pluginDirs = &dir;
}

void PluginManager::setPlugLists(QHash<int, QString> *files, QHash<int, LoaderInterface *> *list, QHash<QString, int> *proto)
{
    plugfiles = files;
    pluglist = list;
    plugproto = proto;
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
            LoaderInterface *ldr = qobject_cast<LoaderInterface*>(plug.instance());
            if(logmodel) logmodel->appendLog(LInterface::MT_INFO,tr("Плагин %1 версия %2-%3 ('%4') загружен.").arg(pluginInfo(ldr,"Plugin"),pluginInfo(ldr,"Version"),pluginInfo(ldr,"Build date"),pluginDirs->value(i)+"/"+plg.value(y)),QString());
            pluglist->insert(pluglist->size()+1,ldr);
            plugfiles->insert(pluglist->size(),pluginDirs->value(i)+"/"+plg.value(y));
            if(logmodel) connect(ldr,SIGNAL(messageAvailable(int,int,QString,QString)),this,SLOT(appendLog(int,int,QString,QString)),Qt::QueuedConnection);

            QStringList protocols = ldr->protocols();
            for(int x=0; x<protocols.size(); x++)
            {
                if(plugproto->value(protocols.value(x)))continue;
                plugproto->insert(protocols.value(x),pluglist->size());
                ldr->setMaxSectionsOnTask(*max_threads);
                ldr->setDownSpeed(*down_speed*1024/8);
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

void PluginManager::setDefaultSettings(const int &tasks, const int &threads, const qint64 &speed)
{
    max_tasks = &tasks;
    max_threads = &threads;
    down_speed = &speed;
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

void PluginManager::appendLog(int id_task, int ms_type, const QString &title, const QString &more)
{
    LoaderInterface *ldr = qobject_cast<LoaderInterface*>(sender());
    if(!ldr) return;
    int new_id = pluglist->key(ldr) * 100 + id_task;
    emit messageAvailable(ms_type,title,more);
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

        in >> len; //считываем размер строки пути до файла плагина
        if(!len) break;
        inbuf.clear();
        inbuf.resize(len);
        in.readRawData(inbuf.data(),len); //считываем строку пути до файла плагина
        QString filepath = inbuf;
        if(filepath == "") break; //если не удалось считать, то выходим

        in >> len;
        int id = plugfiles->key(filepath);
        if(!pluglist->contains(id) || !pluglist->value(id)) continue;
        plugproto->insert(proto,id);
    }
    if(!plugproto->size()) *plugproto = last_state;
}

void PluginManager::setLogModel(LogTreeModel *model)
{
    logmodel = model;
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
        QString filepath = plugfiles->value(id); //путь до плагина
        if(filepath == "") continue;

        stat << filepath.toAscii().size(); //размер строки пути файла плагина
        stat.writeRawData(filepath.toAscii().data(),filepath.toAscii().size()); //путь до файла плагина
    }
    stat << (int) 0;

    return out;
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
