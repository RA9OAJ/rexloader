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

#include "nixnotifyplugin.h"

QDBusInterface NixNotifyPlugin::dbusNotification("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", QDBusConnection::sessionBus());

NixNotifyPlugin::NixNotifyPlugin()
{
    resetImage();
    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications",
                                          "ActionInvoked",
                                          this,SLOT(sendActData(uint,QString)));

    QDBusConnection::sessionBus().connect("org.freedesktop.Notifications",
                                          "/org/freedesktop/Notifications",
                                          "org.freedesktop.Notifications",
                                          "NotificationClosed",
                                          this,SLOT(notifIsClosed(uint)));
}

NixNotifyPlugin::~NixNotifyPlugin()
{
    closeNotify(-1);
}

QStringList NixNotifyPlugin::pluginInfo() const
{
    QStringList pinfo;
    pinfo << QString("Plugin: ") + tr("NixNotify");
    pinfo << QString("Authors: ") + tr("Sarvaritdino R.");
    pinfo << QString("Place: Russia, Barabinsk, 2011-2013");
    pinfo << QString("Build date: ") + QString("20013-02-08");
    pinfo << QString("Version: ") + QString("0.1");
    pinfo << QString("Contacts: mailto:ra9oaj@mail.ru");
    pinfo << QString("Lic: GNU/GPL v3");
    pinfo << QString("Description: ") + tr("Плагин отображения системных уведомлений через DBus.");
    return pinfo;
}

void NixNotifyPlugin::notify(const QString &app, const QString &title, const QString &msg, int timeout, int type, const QStringList &actions, QImage *image)
{
    Q_UNUSED(type)
    QVariantList args;
    args << app;
    args << QVariant(QVariant::UInt);
    args << QVariant("");
    args << title;
    args << msg;
    args << actions;

    QVariantMap map;
    if(image)
    {
        iiibiiay img(image);
        map.insert("icon_data",QVariant(iiibiiay::tid,&img));
    }

    args << map;

    args << (timeout != 0 ? timeout * 1000 : 5000);

    QDBusMessage ret = dbusNotification.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
    if(ret.arguments().size())
        nlist.append(ret.arguments().value(0).toUInt());
}

void NixNotifyPlugin::setImage(int type, QImage *img)
{
    if(!img)
        return;

    QImage newim = img->scaledToWidth(50, Qt::FastTransformation);
    images.insert(type, iiibiiay(&newim));
}

void NixNotifyPlugin::resetImage(int type)
{
    Q_UNUSED(type)
    return;
}

void NixNotifyPlugin::sendActData(unsigned int id, const QString &act)
{
    emit notifyActionData(id, act);
    closeNotify(id);
}

void NixNotifyPlugin::closeNotify(unsigned int id)
{
    if(id == 0 && nlist.size())
    {
        unsigned int cid = 0;
        foreach(cid,nlist)
            closeNotify(cid);
        return;
    }
    dbusNotification.call(QDBus::AutoDetect, "CloseNotification", id);
}

void NixNotifyPlugin::notifIsClosed(unsigned int id)
{
    nlist.removeAll(id);
}

const int iiibiiay::tid = qDBusRegisterMetaType<iiibiiay>();

iiibiiay::iiibiiay()
{
}

iiibiiay::iiibiiay(QImage *img)
{
    width = img->width();
    height = img->height();
    bytesPerLine = img->bytesPerLine();
    alpha = img->hasAlphaChannel();
    channels = (img->isGrayscale()?1:(alpha?4:3));
    bitPerPixel = img->depth()/channels;
    data.append((char*)img->bits(),img->numBytes());
}

QDBusArgument &operator<<(QDBusArgument &a, const iiibiiay &i)
{
    a.beginStructure();
    a << i.width <<i.height << i.bytesPerLine << i.alpha << i.bitPerPixel << i.channels << i.data;
    a.endStructure();
    return a;
}

const QDBusArgument &operator>>(const QDBusArgument &a,  iiibiiay &i)
{
    a.beginStructure();
    a >> i.width >> i.height >> i.bytesPerLine >> i.alpha >> i.bitPerPixel >> i.channels >> i.data;
    a.endStructure();
    return a;
}

Q_EXPORT_PLUGIN2(NixNotifyPlugin, NixNotifyPlugin)
