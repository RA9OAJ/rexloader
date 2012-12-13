/*
Project: REXLoader (Downloader, interface for notification plugins), Source file: NotifInterface.h
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

#ifndef NOTIFINTERFACE_H
#define NOTIFINTERFACE_H

class QObject;
class QString;
class QStringList;
class QImage;
class QUrl;

class NotifInterface : public QObject
{
public:
    enum NotifyType
    {
        WARNING,
        ERROR,
        INFO,
        QUESTION,
        USER
    };

    virtual ~NotifInterface(){}

    virtual QStringList pluginInfo() const = 0; //возвращает данные о плагине и его авторах
    virtual void notify(const QString &app, const QString &title, const QString &msg, int timeout, int type = INFO, const QStringList &actions = QStringList(), QImage *image=0)=0; //выводит уведомление
    virtual void setImage(int type, QImage *img)=0;
    virtual void resetImage(int type = -1)=0;

signals:
    virtual void notifyActionData(unsigned int id, const QString &actname)=0;
};

Q_DECLARE_INTERFACE(NotifInterface,"local.rav.RExLoader.NotifInterface/0.1a")
#endif // NOTIFINTERFACE_H
