/*
Project: REXLoader (Downloader, plugin: HttpLoader), Source file: httploader.h
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

#ifndef HTTPLOADER_H
#define HTTPLOADER_H

#include <QObject>
#include <QtGui/QtGui>
#include <QMutex>
#include <QUrl>
#include <QFileInfo>
#include <QTime>
#include <QTimer>
#include <QTranslator>
#include <stdio.h>
#include <QNetworkProxy>
#include "httpsection.h"
#include "LoaderInterface.h"

class Task{
public:
    Task(){
        url.clear();
        MIME.clear();
        filepath.clear();
        referer.clear();
        size = 0;
        sections_cnt = 0;
        for(int i=0; i<14; i++)
            map[i]=0;
        accept_ranges=false;
        status = LInterface::ON_PAUSE;
        errors_cnt = 0;
        error_number = 0;
        _fullsize_res = true;
        _maxSections = 1;
        last_size = 0;
        cur_speed = 0;
        proxy_auth.clear();
        proxy.clear();
        proxy_type = LInterface::PROXY_NOPROXY;
    }

    ~Task()
    {
        url.clear();
        MIME.clear();
        filepath.clear();
        referer.clear();
    }

    qint64 totalLoad() const
    {
        return (map[1]+map[3]+map[5]+map[7]+map[9]+map[11]);
    }

    QUrl url;
    QUrl proxy; //адрес прокси сервера
    QString proxy_auth; //данные для аутентификации на прокси в base64
    QString MIME;
    QString filepath; //путь к локальному файлу
    QString authData; //данные авторизации
    QString referer;
    qint64 size; //размер задания
    QDateTime last_modif; //дата прошлой модификации
    QDateTime err_modif; //текущая дата модификации файла на удаленном сервере
    qint64 map[14]; //карта секций
    int sections_cnt; //кол-во активных секций
    LInterface::ProxyType proxy_type; //тип прокси сервера
    bool accept_ranges; //флаг докачки
    bool _fullsize_res; //флаг выделения места под весь файл
    QHash<int, HttpSection*>sections; //хэш указателей на секции закачки
    QHash<int, QUrl> mirrors; //список зеркал (если key<0, то это альтернатива для основного URL)
    int status; //статус закачки
    int errors_cnt; //общее кол-во ошибок на закачку
    int error_number;
    int _maxSections;
    QTime watcher;
    qint64 last_size;
    qint64 cur_speed;
};

class HttpLoader : public LoaderInterface
{
    Q_OBJECT
    Q_INTERFACES(LoaderInterface)
public:
    explicit HttpLoader(QObject *parent = 0);
    ~HttpLoader();

    QStringList protocols() const; //возвращает список поддерживаемых протоколов
    QStringList pluginInfo() const; //возвращает данные о плагине и его авторах

    int addTask(const QUrl &_url); //возвращает номер задания при удачной попытке добавления задания на закачку, иначе 0
    void startDownload(int id_task); //стартует задание id_task
    void stopDownload(int id_task); //останавливает задание id_task
    void deleteTask(int id_task); //останавливает задание id_task и удаляет его
    void setTaskFilePath(int id_task, const QString &_path); //устанавливает имя файла и путь для сохранения
    void setDownSpeed(long long int _spd); //устанавливает максимальную скорость скачивания на все задания
    void setMaxSectionsOnTask(int _max); //устанавливает максимальное количество секций на задание
    void setAuthorizationData(int id_task, const QString &data_base64); //устанавливает логин/пароль в base64 для вэб-авторизации
    void setUserAgent(const QString &_uagent); //устанавливает идентификационные данные пользовательского агента (например: Opera)
    void setReferer(int id_task,const QString &uref); //устанавливает реферера для id_task
    void setAttemptInterval(const int sec); //устанавливает интервал между повторными попытками скачать секцию/задание
    void setMaxErrorsOnTask(const int max); //устанавливает максимальное количество ошибок при выполнении задания
    void setRetryCriticalError(const bool flag = false); //указывает, повторять ли попытки закачать при критических ошибках
    long long int totalSize(int id_task) const; //возвращает общий размер задания id_task
    long long int sizeOnSection(int id_task, int _sect_num) const; //возвращает общий размер секции _sect_num для задания id_task
    long long int totalLoadedOnTask(int id_task) const; //возвращает объем закачанного для задания целиком
    long long int totalLoadedOnSection(int id_task, int _sect_num) const; //возвращает объем скачанного в секции _sect_num для задания id_task
    long long int totalDownSpeed() const; //общая скорость скачивания
    long long int downSpeed(int id_task) const; //скорость скачивания задания id_task
    int taskStatus(int id_task) const; //возвращает состояние задания id_task
    int errorNo(int id_task) const; //возвращает код ошибки задания
    int loadTaskFile(const QString &_path); //загружает метаданные задания из указанного файла и возвращает идентификатор задания (при неудаче - 0)
    int countSectionTask(int id_task) const; //возвращает количество активных секций в задании
    int countTask() const; //возвращает количество назначенных заданий
    bool acceptRanges(int id_task) const; //возвращает true, если возможна докачка задания id_task, иначе false
    QString taskFilePath(int id_task) const; //вохвращает полный путь к локальному файлу
    QString errorString(int _err) const; //возвращает строку по заданному коду ошибки
    QString statusString(int _stat) const; //возвращает строку статуса по заданному коду
    void setProxy(int id_task, const QUrl &_proxy, LInterface::ProxyType _ptype, const QString &data_base64); //устанавливает прокси
    QTranslator* getTranslator(const QLocale &locale); //возвращает указатель на транслятор для указанной локали, либо 0 при отсутствии транслятора

signals:
    void sheduleImpulse(); //сигнал генериться с интервалом шедулера
    void messageAvailable(int id_task, int id_sect, int ms_type, const QString &title, const QString &more); //сигнал сообщает о наличии служебных сообщений для задания id_task
    void needAuthorization(int id_task); //сигнал сообщает о необходимости авторизации

protected:
    void mathSpeed(); //перерасчитывает скорость секций всех закачек;
    void addDeleteQueue(HttpSection* sect_); //добавляет секцию в очередь на удаление
    void scanDelQueue(); //сканирует и удаляет из очереди секции, внутренние объекты которых высвободили занятую память

protected slots:
    void addSection(); //добавляет секцию к вызвавшему заданию
    void addSection(int id_task); //слот для периодического повторения попыток добавления секции
    void sheduler(); //шедулер для всех заданий
    void sectError(int _errno); //обработчик ошибок секции
    void setTotalSize(qint64 _sz); //устанавливает общий размер скачиваемого файла
    void redirectToUrl(const QString &_url); //обработчик перенаправлений секций
    void setMIME(const QString &_mime); //устанавливает MIME-тип файла для задания
    void acceptSectionData(); //фиксирует удачное скачивание данных для секции в карте задания
    void mismatchOfDates(const QDateTime &_last, const QDateTime &_cur); //уточняет различие в датах модификации
    void sectionCompleted(); //анализирует результат работы секции и удаляет завершенную секцию
    void syncFileMap(Task* _task); //записывает данные о задании в файл
    void acceptQuery();
    void acceptRang();
    void addInAQueue(); //распознает задачу-источник и ставит в очередь на анализ прогресса закачки
    void addRetSection();
    void addMessage(int ms_type,const QString &message, const QString &more); //слот для приема сообщений о событиях от секций заданий

    Task* getTaskSender(QObject* _sender) const;

private:
    QHash<int, Task*> *task_list; //хэш заданий по ключам
    QHash<HttpSection*, int> *sections; //хэш ключей заданий по ссылкам на секции
    QList<int> *squeue; //очередь на создание нового потока
    QList<int> *dqueue; //очередь на удаление задачи
    QList<HttpSection*> *del_queue; //очередь на удаление отработанных секций
    QList<QObject*> *aqueue; //очередь заданий на обработку в методе acceptRange();
    int maxTaskNum; //хранится максимальный номер существующего задания
    int maxErrors;
    int maxSections;
    int attempt_interval; //интервал повтора
    QString uAgent;
    qint64 speed;

    bool shedule_flag;
    bool fullsize_res; //признак выделения/не выделения места под весь файл
    bool ignore_critical;

    QTranslator *translator;
};

#endif // HTTPLOADER_H
