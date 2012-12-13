/*
Project: REXLoader (Downloader, DBUs based notification plugins), Source file: nixnotifyplugin.cpp
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
}

NixNotifyPlugin::~NixNotifyPlugin()
{
}

QStringList NixNotifyPlugin::pluginInfo() const
{
    return QStringList();
}

void NixNotifyPlugin::notify(const QString &app, const QString &title, const QString &msg, int timeout, int type, const QStringList &actions, QImage *image)
{
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

    dbusNotification.callWithArgumentList(QDBus::AutoDetect, "Notify", args);
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
    return;
}

void NixNotifyPlugin::sendActData(unsigned int id, const QString &act)
{
    emit notifyActionData(id, act);
    qDebug()<<id<<act;
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
