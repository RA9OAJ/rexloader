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
    void notify(QString app, QString title, QString msg, unsigned int timeout, int type = INFO, QStringList actions = QStringList(), QImage *image = 0); //выводит уведомление
    void setImage(int type, QImage *img);
    void resetImage(int type = -1);

signals:
    void notifyActionData(unsigned int id, const QString &actname);

protected slots:
    void sendActData(unsigned int id, const QString &act);

private:
    static QDBusInterface dbusNotification;
    QMap<int, iiibiiay> images;
};

#endif // NIXNOTIFYPLUGIN_H
