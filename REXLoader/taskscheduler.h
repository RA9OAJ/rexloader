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

#ifndef TASKSCHEDULER_H
#define TASKSCHEDULER_H

#include <QObject>
#include <QHash>
#include <QTimer>

#include "pluginmanager.h"

class TaskScheduler : public QObject
{
    Q_OBJECT
public:
    explicit TaskScheduler(QObject *parent = 0);
    ~TaskScheduler();
    void setQueryManager(PluginManager *mgr);
    
signals:
    
public slots:
    void showWindow();

protected slots:
    void startTasks();
    void stopTasks();

signals:
    void needExecQuery(const QString &qry);

private:
    QHash<int,QTimer*> starttmrs;
    QHash<int,QTimer*> endtmrs;

};

#endif // TASKSCHEDULER_H
