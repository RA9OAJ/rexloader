/*
Copyright (C) 2010-2014  Sarvaritdinov R.

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

#include "ftploader.h"

#define P_VERSION "0.1a.0"
#define P_BUILD_DATE "2014-01-19"

FtpLoader::FtpLoader(QObject *parent)
{
}

QStringList FtpLoader::protocols() const
{
    return QStringList() << "ftp";
}

QStringList FtpLoader::pluginInfo() const
{
    QStringList pinfo;
    pinfo << QString("Plugin: ") + tr("FtpLoader");
    pinfo << QString("Authors: ") + tr("Sarvaritdinov R.");
    pinfo << QString("Place: Russia, Barabinsk, 2011-2014");
    pinfo << QString("Build date: ") + QString(P_BUILD_DATE);
    pinfo << QString("Version: ") + QString(P_VERSION);
    pinfo << QString("Contacts: mailto:ra9oaj@mail.ru");
    pinfo << QString("Lic: GNU/GPL v3");
    pinfo << QString("Description: ") + tr("Плагин для скачивания файлов по протоколу FTP.");
    return pinfo;
}

int FtpLoader::addTask(const QUrl &_url)
{
    return 0;
}

void FtpLoader::startDownload(int id_task)
{
}

void FtpLoader::stopDownload(int id_tsk)
{
}

void FtpLoader::deleteTask(int id_task)
{
}

void FtpLoader::setTaskFilePath(int id_task, const QString &_path)
{
}

void FtpLoader::setDownSpeed(long long _spd)
{
}

void FtpLoader::setMaxSectionsOnTask(int _max)
{
}

void FtpLoader::setAuthorizationData(int id_task, const QString &data_base64)
{
}

void FtpLoader::setUserAgent(const QString &_uagent)
{
}

void FtpLoader::setReferer(int id_task, const QString &uref)
{
}

void FtpLoader::setAttemptInterval(const int sec)
{
}

void FtpLoader::setMaxErrorsOnTask(const int max)
{
}

void FtpLoader::setRetryCriticalError(const bool flag)
{
}

long long FtpLoader::totalSize(int id_task) const
{
    return 0;
}

long long FtpLoader::sizeOnSection(int id_task, int _sect_num) const
{
    return 0;
}

long long FtpLoader::totalLoadedOnTask(int id_task) const
{
    return 0;
}

long long FtpLoader::totalLoadedOnSection(int id_task, int _sect_num) const
{
    return 0;
}

long long FtpLoader::totalDownSpeed() const
{
    return 0;
}

long long FtpLoader::downSpeed(int id_task) const
{
    return 0;
}

int FtpLoader::taskStatus(int id_task) const
{
    return 0;
}

int FtpLoader::errorNo(int id_task) const
{
    return 0;
}

int FtpLoader::loadTaskFile(const QString &_path)
{
    return 0;
}

int FtpLoader::countSectionTask(int id_task) const
{
    return 0;
}

int FtpLoader::countTask() const
{
    return 0;
}

bool FtpLoader::acceptRanges(int id_task) const
{
    return false;
}

QString FtpLoader::mimeType(int id_task) const
{
    return QString();
}

QString FtpLoader::taskFilePath(int id_task) const
{
    return QString();
}

QString FtpLoader::errorString(int _err) const
{
    return QString();
}

QString FtpLoader::statusString(int _stat) const
{
    return QString();
}

void FtpLoader::setProxy(int id_task, const QUrl &_proxy, LInterface::ProxyType _ptype, const QString &data_base64)
{
}

void FtpLoader::setAdvancedOptions(int id_task, const QString &options)
{
}

QTranslator *FtpLoader::getTranslator(const QLocale &locale)
{
    return 0;
}

QWidget *FtpLoader::widgetSettings(const QString &file_path)
{
    return 0;
}

Q_EXPORT_PLUGIN2(FtpLoader, FtpLoader)
