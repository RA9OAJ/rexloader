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

#ifndef NIXNOTIFYPLUGIN_H
#define NIXNOTIFYPLUGIN_H

#include <QObject>
#include <QStringList>
#include <QVariant>
#include <QByteArray>
#include <QMap>
#include <QtDBus>
#include <QImage>

#include "../NotifInterface.h"

struct iiibiiay
{
    iiibiiay(QImage *img);
    iiibiiay();
    static const int tid;
    int width;
    int height;
    int bytesPerLine;
    bool alpha;
    int channels;
    int bitPerPixel;
    QByteArray data;
};
Q_DECLARE_METATYPE(iiibiiay)

class NixNotifyPlugin : public NotifInterface
{
    Q_OBJECT
    Q_INTERFACES(NotifInterface)
public:
    explicit NixNotifyPlugin();
    ~NixNotifyPlugin();

    QStringList pluginInfo() const; //возвращает данные о плагине и его авторах
    void notify(const QString &app, const QString &title, const QString &msg, int timeout, int type = INFO, const QStringList &actions = QStringList(), QImage *image = 0); //выводит уведомление
    void setImage(int type, QImage *img);
    void resetImage(int type = -1);

signals:
    void notifyActionData(unsigned int id, const QString &actname);

protected slots:
    void sendActData(unsigned int id, const QString &act);
    void closeNotify(unsigned int id);
    void notifIsClosed(unsigned int id);

private:
    static QDBusInterface dbusNotification;
    QMap<int, iiibiiay> images;
    QList<unsigned int> nlist;
};

#endif // NIXNOTIFYPLUGIN_H
