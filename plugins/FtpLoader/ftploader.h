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

#ifndef FTPLOADER_H
#define FTPLOADER_H

#include <QObject>
#include <QtGui/QtGui>
#include "../LoaderInterface.h"


class FtpLoader : public LoaderInterface
{
    Q_OBJECT
    Q_INTERFACES(LoaderInterface)
    
public:
    explicit FtpLoader(QObject *parent = 0);

    virtual QStringList protocols() const; //возвращает список поддерживаемых протоколов
    virtual QStringList pluginInfo() const; //возвращает данные о плагине и его авторах

    virtual int addTask(const QUrl &_url); //возвращает номер задания при удачной попытке добавления задания на закачку, иначе 0
    virtual void startDownload(int id_task); //стартует задание id_task
    virtual void stopDownload(int id_tsk); //останавливает задание id_task
    virtual void deleteTask(int id_task); //останавливает задание id_task и удаляет его
    virtual void setTaskFilePath(int id_task, const QString &_path); //устанавливает имя файла и путь для сохранения
    virtual void setDownSpeed(long long int _spd); //устанавливает максимальную скорость скачивания на все задания
    virtual void setMaxSectionsOnTask(int _max); //устанавливает максимальное количество секций на задание
    virtual void setAuthorizationData(int id_task, const QString &data_base64); //устанавливает логин/пароль в base64 для вэб-авторизации
    virtual void setUserAgent(const QString &_uagent); //устанавливает идентификационные данные пользовательского агента (например: Opera)
    virtual void setReferer(int id_task,const QString &uref); //устанавливает реферера для id_task
    virtual void setAttemptInterval(const int sec); //устанавливает интервал между повторными попытками скачать секцию/задание
    virtual void setMaxErrorsOnTask(const int max); //устанавливает максимальное количество ошибок при выполнении задания
    virtual void setRetryCriticalError(const bool flag); //указывает, повторять ли попытки закачать при критических ошибках
    virtual long long int totalSize(int id_task) const; //возвращает общий размер задания id_task
    virtual long long int sizeOnSection(int id_task, int _sect_num) const; //возвращает общий размер секции _sect_num для задания id_task
    virtual long long int totalLoadedOnTask(int id_task) const; //возвращает объем закачанного для задания целиком
    virtual long long int totalLoadedOnSection(int id_task, int _sect_num) const; //возвращает объем скачанного в секции _sect_num для задания id_task
    virtual long long int totalDownSpeed() const; //общая скорость скачивания
    virtual long long int downSpeed(int id_task) const; //скорость скачивания задания id_task
    virtual int taskStatus(int id_task) const; //возвращает состояние задания id_task
    virtual int errorNo(int id_task) const; //возвращает код ошибки задания
    virtual int loadTaskFile(const QString &_path); //загружает метаданные задания из указанного файла и возвращает идентификатор задания (иначе - 0)
    virtual int countSectionTask(int id_task) const; //возвращает количество активных секций в задании
    virtual int countTask() const; //возвращает количество назначенных заданий
    virtual bool acceptRanges(int id_task) const; //возвращает true, если возможна докачка задания id_task, иначе false
    virtual QString mimeType(int id_task) const; //возвращает mime тип задачи
    virtual QString taskFilePath(int id_task) const; //вохвращает полный путь к локальному файлу
    virtual QString errorString(int _err) const; //возвращает строку по заданному коду ошибки
    virtual QString statusString(int _stat) const; //возвращает строку статуса по заданному коду
    virtual void setProxy(int id_task, const QUrl &_proxy, LInterface::ProxyType _ptype, const QString &data_base64); //устанавливает прокси
    virtual void setAdvancedOptions(int id_task, const QString &options); //передает загрузчику дополнительные параметры
    virtual QTranslator* getTranslator(const QLocale &locale); //возвращает указатель на транслятор для указанной локали, либо 0 при отсутствии транслятора
    virtual QWidget* widgetSettings(const QString &file_path); //возвращает указатель на виджет настроек плагина (file_path - путь до папки сохранения настрое плагина), или 0 при отсутствии настроек

signals:
    void sheduleImpulse(); //сигнал генериться с интервалом шедулера
    virtual void messageAvailable(int id_task, int id_sect, int ms_type, const QString &title, const QString &more); //сигнал сообщает о наличии служебных сообщений для задания id_task
    virtual void needAuthorization(int id_task, const QUrl &url); //сигнал сообщает о необходимости авторизации
};

#endif // FTPLOADER_H
