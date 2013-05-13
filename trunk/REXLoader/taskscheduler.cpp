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

#include "taskscheduler.h"

TaskScheduler::TaskScheduler(QObject *parent) :
    QObject(parent)
{
}

TaskScheduler::~TaskScheduler()
{
}

void TaskScheduler::setQueryManager(PluginManager *mgr)
{
    connect(this,SIGNAL(needExecQuery(QString)),mgr,SLOT(exeQuery(QString)));
}

void TaskScheduler::showWindow()
{
}

void TaskScheduler::startTasks()
{
    QTimer *tmr = qobject_cast<QTimer*>(sender());
    int plane_id = starttmrs.key(tmr,-1);
    if(plane_id < 0)
        return;

    QString query("UPDATE tasks SET status=-100 WHERE status=-101 AND plane_id="+plane_id);
    needExecQuery(query);
}

void TaskScheduler::stopTasks()
{

}

