#include "systemiconswrapper.h"

QHash<QString,QHash<int,QString> > SystemIconsWrapper::_cur_theme;
QString SystemIconsWrapper::default_theme = QString();
bool SystemIconsWrapper::_cached = false;
QHash<QString,QPixmap>  SystemIconsWrapper::_cache;
QHash<QString,QString> SystemIconsWrapper::_themes;

SystemIconsWrapper::SystemIconsWrapper(QObject *parent) :
    QObject(parent)
{
}

QIcon SystemIconsWrapper::icon(const QString &name, int size, const QString &alt_name)
{
    return QIcon(pixmap(name,size,alt_name));
}

QImage SystemIconsWrapper::image(const QString &name, int size, const QString &alt_name)
{
    return pixmap(name,size,alt_name).toImage();
}

QPixmap SystemIconsWrapper::pixmap(const QString &name, int size, const QString &alt_name)
{
#ifdef Q_OS_LINUX
    QString section, flname, iconpath, ext, cache_key;
    cache_key = QString("%1\r\n%2").arg(name,QString::number(size));

    if(_cached && _cache.contains(cache_key))
        return _cache.value(cache_key);

    if(name.indexOf("/") > 0)
    {
        section = name.split("/").value(0);
        flname = name.split("/").value(1);
    }
    else flname = name;

    if(default_theme.isEmpty())
        setTheme("default");

    QString theme_root = default_theme;
    if(default_theme.toLower() == "default")
        theme_root = defaultUserTheme();

    if(!theme_root.isEmpty())
    {
        if(theme_root.mid(theme_root.size() - 1) != "/")
            theme_root += "/";

        //Поиск по папкам с подходящими размерами иконок
        QList<int> sizes = _cur_theme.value(section).keys();
        qSort(sizes);
        QList<int>::iterator it = sizes.begin();

        while(it != sizes.end())
        {
            QString cur_path = theme_root + _cur_theme.value(section).value(*it) + "/";
            ext = fileSuffix(cur_path);
            cur_path += flname + ext;

            if(QFile::exists(cur_path))
                iconpath = cur_path;

            if(*it >= size && iconpath == cur_path)
                break;

            ++it;
        }

        if(!QFile::exists(iconpath))
            iconpath = alt_name;

        if(_cached && !_cache.contains(cache_key))
            _cache.insert(cache_key,QPixmap(iconpath).scaledToWidth(size,Qt::SmoothTransformation));

        return QPixmap(iconpath).scaledToWidth(size,Qt::SmoothTransformation);
    }
#endif

    return QPixmap(alt_name).scaledToWidth(size,Qt::SmoothTransformation);
}

QList<QPair<QString, QString> > SystemIconsWrapper::systemThemes()
{
    _themes.clear();
    QList<QPair<QString,QString> > sys_thems_lst;

#ifdef Q_OS_LINUX
    QString sys_path = "/usr/share/icons";
    QString home_path = QDir::homePath() + "/.kde/share/icons";
    QDir sys_dir(sys_path);
    QFileInfoList fl_lst = sys_dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    sys_dir.setPath(home_path);
    fl_lst += sys_dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    foreach(QFileInfo flinfo, fl_lst)
    {
        QString cur_name = nameTheme(flinfo.absoluteFilePath());
        if(!cur_name.isEmpty())
        {
            sys_thems_lst.append(QPair<QString,QString>(cur_name,flinfo.absoluteFilePath()));
            _themes.insert(cur_name,flinfo.absoluteFilePath());
        }
    }
#endif

    return sys_thems_lst;
}

void SystemIconsWrapper::setTheme(const QString &path)
{
    _cache.clear();
    _cur_theme.clear();
    default_theme = path;
    QDir theme_path;

    if(path == "default")
        theme_path.setPath(defaultUserTheme());
    else theme_path.setPath(path);

    QSettings index(theme_path.absolutePath() + "/index.theme",QSettings::IniFormat);
    index.beginGroup("Icon Theme");
    QStringList folders = index.value("Directories").toStringList();
    index.endGroup();

    foreach (QString cur_group, folders)
    {
        index.beginGroup(cur_group);
        int size = index.value("Size").toInt();
        if(cur_group.indexOf("scalable") != -1)
            size = 0;
        _cur_theme[index.value("Context").toString().toLower()].insert(size,cur_group);
        index.endGroup();
    }
}

void SystemIconsWrapper::cachedIcons(bool enabled)
{
    _cached = enabled;
    _cache.clear();
}

QString SystemIconsWrapper::defaultSystemTheme()
{
    QString tsystem;

#ifdef Q_OS_LINUX
    QDir sys_dir("/usr/share/icons");
    QFileInfoList fl_lst = sys_dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    foreach(QFileInfo flinfo, fl_lst)
        if(flinfo.isSymLink())
            tsystem = flinfo.symLinkTarget();
#endif

    return tsystem;
}

QString SystemIconsWrapper::defaultUserTheme()
{
#ifdef Q_OS_LINUX
    QString cfg_path = QDir::homePath() + "/.kde/share/config/kdeglobals";
    QString theme_folder;
    if(QFile::exists(cfg_path))
    {
        QSettings settings(cfg_path,QSettings::IniFormat);
        settings.beginGroup("Icons");
        theme_folder = settings.value("Theme").toString();
        settings.endGroup();

        QString usr_root = "/usr/share/icons/";
        QString home_root = QDir::homePath() + "/.kde/share/icons/";

        if(QFile::exists(usr_root + theme_folder))
            return usr_root + theme_folder;

        if(QFile::exists(home_root + theme_folder))
            return home_root + theme_folder;
    }
#endif

    return defaultSystemTheme();
}

QString SystemIconsWrapper::theme()
{
    if(default_theme.isEmpty() || default_theme.toLower() == "default")
        return QString("default");

    return default_theme;
}

QString SystemIconsWrapper::pathToTheme(const QString &theme_name)
{
    if(_themes.isEmpty())
        systemThemes();

    return _themes.value(theme_name);
}

QString SystemIconsWrapper::nameTheme(const QString &path)
{
    QString tname;

#ifdef Q_OS_LINUX
    if(QFile::exists(path + "/index.theme"))
    {
        QSettings sett(path + "/index.theme", QSettings::IniFormat);
        sett.beginGroup("Icon Theme");
        QStringList str_lst = sett.value("Directories",QStringList()).toStringList();
        if(!str_lst.value(0).isEmpty())
            tname = sett.value("Name").toString();
        sett.endGroup();
    }
#endif

    return tname;
}

QString SystemIconsWrapper::fileSuffix(const QString &path)
{
    QString ext;
    QDir dir(path);
    QFileInfoList fllst = dir.entryInfoList(QDir::Files);
    if(!fllst.isEmpty())
    {
        ext = ".";
        ext += fllst.value(0).suffix();
    }

    return ext;
}
