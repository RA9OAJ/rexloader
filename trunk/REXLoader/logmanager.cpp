/*
Project: REXLoader (Downloader), Source file: logmanager.cpp
Copyright (C) 2012  Sarvaritdinov R.

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

#include "logmanager.h"

LogManager::LogManager(QObject *parent) :
    QObject(parent)
{
    _max_str_count = 1000;
    _tabwidget = 0;
    QList<LogTreeModel*> lst;
    lst << new LogTreeModel(this);
    lst.value(0)->setMaxStringsCount(_max_str_count);
    loglist.insert(-1, lst);
}

LogManager::~LogManager()
{
}

LogTreeModel *LogManager::model(int table_id, int id_sect) const
{
    if(!loglist.contains(table_id)) return 0;
    QList<LogTreeModel*> lst = loglist.value(table_id);
    if(lst.isEmpty() || lst.size() < id_sect) return 0;
    return lst.value(id_sect ? id_sect-1 : id_sect,0);
}

void LogManager::setTabWidget(QTabWidget *widget)
{
    if(_tabwidget)
        while(_tabwidget->count() > 0)
            _tabwidget->removeTab(0);

    _tabwidget = widget;
    manageTabs(_cur_table_id);
}

void LogManager::appendLog(int table_id, int id_sect, int mtype, const QString &title, const QString &more)
{
    if(!loglist.contains(table_id))
    {
        QList<LogTreeModel*> lst;
        loglist.insert(table_id,lst);
    }

    while(loglist[table_id].size() < id_sect)
    {
        LogTreeModel *mdl = new LogTreeModel(this);
        mdl->setMaxStringsCount(_max_str_count);
        loglist[table_id].append(mdl);
        if(_cur_table_id == table_id && table_id > -1)
            manageTabs(table_id);
    }

    LogTreeModel *mdl = model(table_id,id_sect);
    if(mdl) mdl->appendLog(mtype,title,more);
}

void LogManager::setMaxStringCount(int max)
{
    if(max <= 0) return;
    _max_str_count = max;
}

void LogManager::saveLogToFile(const QString &filename, int table_id)
{
}

void LogManager::loadLogFromFile(const QString &file)
{
}

void LogManager::clearLog(int table_id)
{
}

void LogManager::manageTabs(int table_id)
{
    _cur_table_id = table_id;
    if(!_tabwidget) return;
    if(!_tabwidget->count())
    {
        QWidget *wgt = createTabWidget();
        _tabwidget->addTab(wgt,tr("События приложения"));

        if(!loglist.contains(-1))
        {
            QList<LogTreeModel*> lst;
            LogTreeModel *mdl = new LogTreeModel(this);
            mdl->setMaxStringsCount(_max_str_count);
            lst << mdl;
            loglist.insert(-1,lst);
        }

        QTreeView *view = getTreeView(wgt);
        view->setModel(model());
        view->header()->hide();
        view->hideColumn(1);
        view->hideColumn(2);
        view->hideColumn(3);
    }

    if(!loglist.contains(table_id))
    {
        while(_tabwidget->count() > 1)
        {
            _tabwidget->widget(1)->deleteLater();
            _tabwidget->removeTab(1);
        }
        return;
    }

    QList<LogTreeModel*> lst = loglist.value(table_id);
    LogTreeModel *mdl;
    int i = 1;
    foreach(mdl,lst)
    {
        if(_tabwidget->count() < i+1)
        {
            QWidget *wgt = createTabWidget();
            QTreeView *view = getTreeView(wgt);
            view->setModel(model(table_id,i));
            view->header()->hide();
            view->hideColumn(1);
            view->hideColumn(2);
            view->hideColumn(3);
            _tabwidget->addTab(wgt,tr("Поток %1").arg(QString::number(i)));
            ++i;
            continue;
        }

        QTreeView *view = getTreeView(_tabwidget->widget(i));
        if(view->model() != mdl)
        {
            view->setModel(mdl);
            view->header()->hide();
            view->hideColumn(1);
            view->hideColumn(2);
            view->hideColumn(3);
        }
        ++i;
    }

    while(_tabwidget->count() > i)
    {
        _tabwidget->widget(i)->deleteLater();
        _tabwidget->removeTab(i);
    }
}

QWidget *LogManager::createTabWidget()
{
    QWidget *wgt = new QWidget();
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom,wgt);
    QTreeView *tree = new QTreeView(wgt);
    tree->setObjectName("_LogView_");
    layout->addWidget(tree);
    return wgt;
}

QTreeView *LogManager::getTreeView(QWidget *wgt)
{
    return wgt->findChild<QTreeView*>("_LogView_");
}
