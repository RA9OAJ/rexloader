/*
Project: REXLoader (Downloader, interface for plugins-downloaders), Source file: LoaderInterface.h
Copyright (C) 2011  Sarvaritdinov R.

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

#ifndef LOADERINTERFACE_H
#define LOADERINTERFACE_H

class QString;
class QStringList;
class QUrl;
class QTranslator;
class QLocale;

namespace LInterface{

enum Status{
    NO_SECTION=-3, //нет такой секции
    ERROR_TASK=-2, //при выполнении задания возникла ошибка
    NO_TASK=-1, //нет такого задания
    ON_PAUSE=0, //задание приостановлено
    SEND_QUERY=1, //посылка запроса
    ACCEPT_QUERY=2, //запрос принят
    ON_LOAD=3, //задание на выполнении
    REDIRECT=4, //перенаправление на другой адрес
    FINISHED=5, //выполнено успешно
    STOPPING=6 //задание находится в процессе остановки, инициированной пользователем
};

enum Error{
    NO_ERROR = 0, //нет ошибок
    FILE_NOT_FOUND=1, //файл по указанному URL не найден
    FILE_DATETIME_ERROR=2, //файл был изменен
    FILE_SIZE_ERROR=3, //размер файла не совпадает с ранее переданным
    FILE_CREATE_ERROR=4, //невозможно создать локальный файл
    FILE_WRITE_ERROR=5, //ошибка при записи в локальный файл
    FILE_READ_ERROR=6, //ошибка чтения локального файла
    HOST_NOT_FOUND=7, //удаленный хост не найден
    CONNECT_ERROR=8, //невозможно подключиться к удаленному узлу
    CONNECT_LOST=9, //соединение потеряно
    SERVER_REJECT_QUERY=10, //сервер отверг запрос
    PROXY_NOT_FOUND=11, //прокси не найден
    PROXY_AUTH_ERROR=12, //ошибка аутентификации на прокси
    PROXY_ERROR=13, //ошибка протокола прокси
    PROXY_CONNECT_CLOSE=14, //прокси разорвал соединение до окончания передачи данных
    PROXY_CONNECT_REFUSED=15, //прокси отверг соединение или не доступен
    PROXY_TIMEOUT=16, //таймаут соединения с прокси
    ERRORS_MAX_COUNT=17 //достигнуто предельное кол-во ошибок на задание
};

enum ProxyType{
    PROXY_NOPROXY,
    PROXY_DEFAULT,
    PROXY_SOCKS5,
    PROXY_HTTP
};

}

class LoaderInterface{
public:
    virtual ~LoaderInterface(){};

    virtual QStringList protocols() const = 0; //возвращает список поддерживаемых протоколов
    virtual QStringList pluginInfo() const = 0; //возвращает данные о плагине и его авторах

    virtual int addTask(const QUrl &_url)=0; //возвращает номер задания при удачной попытке добавления задания на закачку, иначе 0
    virtual void startDownload(int id_task)=0; //стартует задание id_task
    virtual void stopDownload(int id_tsk)=0; //останавливает задание id_task
    virtual void deleteTask(int id_task) =0; //останавливает задание id_task и удаляет его
    virtual void setTaskFilePath(int id_task, const QString &_path)=0; //устанавливает имя файла и путь для сохранения
    virtual void setDownSpeed(long long int _spd)=0; //устанавливает максимальную скорость скачивания на все задания
    virtual void setMaxSectionsOnTask(int _max) =0; //устанавливает максимальное количество секций на задание
    virtual void setAuthorizationData(int id_task, const QString &data_base64) =0; //устанавливает логин/пароль в base64 для вэб-авторизации
    virtual void setUserAgent(const QString &_uagent)=0; //устанавливает идентификационные данные пользовательского агента (например: Opera)
    virtual void setReferer(int id_task,const QString &uref)=0; //устанавливает реферера для id_task
    virtual void setAttemptInterval(const int sec)=0; //устанавливает интервал между повторными попытками скачать секцию/задание
    virtual void setMaxErrorsOnTask(const int max)=0; //устанавливает максимальное количество ошибок при выполнении задания
    virtual void setRetryCriticalError(const bool flag)=0; //указывает, повторять ли попытки закачать при критических ошибках
    virtual long long int totalSize(int id_task) const = 0; //возвращает общий размер задания id_task
    virtual long long int sizeOnSection(int id_task, int _sect_num) const = 0; //возвращает общий размер секции _sect_num для задания id_task
    virtual long long int totalLoadedOnTask(int id_task) const = 0; //возвращает объем закачанного для задания целиком
    virtual long long int totalLoadedOnSection(int id_task, int _sect_num) const = 0; //возвращает объем скачанного в секции _sect_num для задания id_task
    virtual long long int totalDownSpeed() const = 0; //общая скорость скачивания
    virtual long long int downSpeed(int id_task) const = 0; //скорость скачивания задания id_task
    virtual int taskStatus(int id_task) const = 0; //возвращает состояние задания id_task
    virtual int errorNo(int id_task) const = 0; //возвращает код ошибки задания
    virtual int loadTaskFile(const QString &_path)=0; //загружает метаданные задания из указанного файла и возвращает идентификатор задания (иначе - 0)
    virtual int countSectionTask(int id_task) const =0; //возвращает количество активных секций в задании
    virtual int countTask() const = 0; //возвращает количество назначенных заданий
    virtual bool acceptRanges(int id_task) const = 0; //возвращает true, если возможна докачка задания id_task, иначе false
    virtual QString taskFilePath(int id_task) const =0; //вохвращает полный путь к локальному файлу
    virtual QString errorString(int _err) const =0; //возвращает строку по заданному коду ошибки
    virtual QString statusString(int _stat) const =0; //возвращает строку статуса по заданному коду
    virtual void setProxy(int id_task, const QUrl &_proxy, LInterface::ProxyType _ptype, const QString &data_base64)=0; //устанавливает прокси
    virtual QTranslator* getTranslator(const QLocale &locale) =0; //возвращает указатель на транслятор для указанной локали, либо 0 при отсутствии транслятора

signals:
    virtual void messageAvailable(int id_task)=0; //сигнал сообщает о наличии служебных сообщений для задания id_task
    virtual void needAuthorization(int id_task)=0; //сигнал сообщает о необходимости авторизации
};


Q_DECLARE_INTERFACE(LoaderInterface,"local.rav.RExLoader/0.1a")
#endif // LOADERINTERFACE_H
