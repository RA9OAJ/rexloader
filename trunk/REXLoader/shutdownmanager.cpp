/*
Project: REXLoader (Downloader), Source file: shutdownmanager.cpp
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

#include "shutdownmanager.h"

ShutdownManager::ShutdownManager(QObject *parent) :
    QObject(parent)
{
    curmode = Shutdown;
}

ShutdownManager::~ShutdownManager()
{
}

ShutdownManager::DEType ShutdownManager::DEDetect()
{
#ifdef Q_OS_UNIX
    {
        QDBusInterface mgr("org.kde.ksmserver", "/KSMServer" , "org.kde.KSMServerInterface");
        QDBusReply<bool> rp = mgr.call("canShutdown");
        if(rp.value()) return KDE4;
    }

#endif

    return Other;
}

bool ShutdownManager::shutdownPC()
{
#ifdef Q_OS_UNIX
    if(DEDetect() == KDE4)
    {
        QDBusInterface mgr("org.kde.ksmserver", "/KSMServer" , "org.kde.KSMServerInterface");
        QDBusReply<bool> rp = mgr.call("canShutdown");
        if(!rp.value())
            return false;
        mgr.call("logout",0,2,2);
    }
    else if(DEDetect() == Gnome)
    {
        QDBusInterface mgr("org.gnome.SessionManager", "/org/gnome/SessionManager" , "org.gnome.SessionManager");
        QDBusReply<bool> rp = mgr.call("CanShutdown");
        if(!rp.value())
            return false;
        mgr.call("RequestShutdown");
    }
    else
    {
        QDBusInterface mgr("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager" , "org.freedesktop.ConsoleKit.Manager", QDBusConnection::systemBus());
        QDBusReply<bool> rp = mgr.call("CanStop");
        if(!rp.value())
            return false;
        mgr.call("Stop");
    }
    return true;
#endif

    QProcess::startDetached("shutdown /s /f /t 00");
    return true;
}

bool ShutdownManager::suspendPC()
{
#ifdef Q_OS_UNIX
    QDBusInterface mgr("org.freedesktop.UPower", "/org/freedesktop/UPower" , "org.freedesktop.UPower", QDBusConnection::systemBus());
    QDBusReply<bool> rp = mgr.call("SuspendAllowed");
    if(!rp.value())
        return false;
    mgr.call("Suspend");

    return true;
#endif

    QProcess::startDetached("shutdown /h /f");
    return true;
}

bool ShutdownManager::hibernatePC()
{
#ifdef Q_OS_UNIX
    QDBusInterface mgr("org.freedesktop.UPower", "/org/freedesktop/UPower" , "org.freedesktop.UPower", QDBusConnection::systemBus());
    QDBusReply<bool> rp = mgr.call("HibernateAllowed");
    if(!rp.value())
        return false;
    mgr.call("Hibernate");

    return true;
#endif

    QProcess::startDetached("shutdown /h /f");
    return true;
}

void ShutdownManager::setMode(ShutdownManager::ShutdownMode mode)
{
    curmode = mode;
}

void ShutdownManager::startShutdown() const
{
    switch(curmode)
    {
    case Hibernate:
        hibernatePC();
        break;

    case Suspend:
        suspendPC();
        break;

    default:
        shutdownPC();
        break;
    }
}
