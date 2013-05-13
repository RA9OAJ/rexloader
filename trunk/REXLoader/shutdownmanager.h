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

#ifndef SHUTDOWNMANAGER_H
#define SHUTDOWNMANAGER_H

#include <QObject>
#include <QProcess>

#ifdef Q_OS_UNIX
#include <QtDBus/QtDBus>
#endif

class ShutdownManager : public QObject
{
    Q_OBJECT
public:
    enum DEType{
        KDE4,
        Gnome,
        Other
    };

    enum ShutdownMode
    {
        Shutdown,
        Suspend,
        Hibernate
    };

    explicit ShutdownManager(QObject *parent = 0);
    ~ShutdownManager();

    static DEType DEDetect();
    static bool shutdownPC();
    static bool suspendPC();
    static bool hibernatePC();

signals:
    
public slots:
    void setMode(ShutdownMode mode);
    void startShutdown() const;

private:
    ShutdownMode curmode;
    DEType detype;
};

#endif // SHUTDOWNMANAGER_H
