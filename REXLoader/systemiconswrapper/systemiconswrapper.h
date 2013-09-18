#ifndef SYSTEMICONSWRAPPER_H
#define SYSTEMICONSWRAPPER_H

#include <QObject>
#include <QSettings>
#include <QIcon>
#include <QHash>
#include <QDir>
#include <QDebug>

class SystemIconsWrapper : public QObject
{
    Q_OBJECT
public:
    explicit SystemIconsWrapper(QObject *parent = 0);
    static QIcon icon(const QString &name, int size = 0, const QString &alt_name = QString());
    static QImage image(const QString &name, int size = 0, const QString &alt_name = QString());
    static QPixmap pixmap(const QString &name, int size = 0, const QString &alt_name = QString());
    static QList<QPair<QString,QString> > systemThemes();
    static void setTheme(const QString &path);
    static void cachedIcons(bool enabled);
    static QString defaultSystemTheme();
    static QString defaultUserTheme();
    static QString theme();
    static QString pathToTheme(const QString &theme_name);
    static QString nameTheme(const QString &path);

protected:
    static QString fileSuffix(const QString &path);

private:
    static QHash<QString,QPixmap> _cache;
    static QHash<QString,QString> _themes;
    static bool _cached;
    static QString default_theme;
    static QHash<QString,QHash<int,QString> > _cur_theme;
};

#endif // SYSTEMICONSWRAPPER_H
