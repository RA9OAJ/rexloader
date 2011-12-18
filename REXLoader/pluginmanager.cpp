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

#include "pluginmanager.h"

PluginManager::PluginManager(QObject *parent) :
    QThread(parent)
{
    pluginDirs = 0;
    plugfiles = 0;
    plugproto = 0;
    max_tasks = 0;
    max_threads = 0;
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
            pluglist->insert(pluglist->size()+1,ldr);
            plugfiles->insert(pluglist->size(),pluginDirs->value(i)+"/"+plg.value(y));

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
    int plug = id_task/100;
    pluglist->value(plug)->startDownload(task);
}

void PluginOperator::stopDownload(int id_task)
{
    if(!pluglist)return;
    int task = id_task%100;
    int plug = id_task/100;
    pluglist->value(plug)->stopDownload(task);
}

void PluginManager::loadLocale(const QLocale &locale)
{
    QList<int> keys = pluglist->keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        QTranslator *translator = pluglist->value(keys.value(i))->getTranslator(locale);
        if(!translator) continue;
        translator->moveToThread(thread());
        qApp->installTranslator(translator);
    }
}


