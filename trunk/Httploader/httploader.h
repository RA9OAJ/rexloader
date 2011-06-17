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
    int status; //статус закачки
    int errors_cnt; //общее кол-во ошибок на закачку
    int error_number;
    int _maxSections;
    QTime watcher;
    qint64 last_size;
    qint64 cur_speed;
};

class HttpLoader : public QObject , public LoaderInterface
{
    Q_OBJECT
    Q_INTERFACES(LoaderInterface)
public:
    explicit HttpLoader(QObject *parent = 0);
    virtual ~HttpLoader();

    virtual QStringList protocols() const; //возвращает список поддерживаемых протоколов
    virtual QStringList pluginInfo() const; //возвращает данные о плагине и его авторах

    virtual int addTask(const QUrl &_url); //возвращает номер задания при удачной попытке добавления задания на закачку, иначе 0
    virtual void startDownload(int id_task); //стартует задание id_task
    virtual void stopDownload(int id_task); //останавливает задание id_task
    virtual void deleteTask(int id_task); //останавливает задание id_task и удаляет его
    virtual void setTaskFilePath(int id_task, const QString &_path); //устанавливает имя файла и путь для сохранения
    virtual void setDownSpeed(long long int _spd); //устанавливает максимальную скорость скачивания на все задания
    virtual void setMaxSectionsOnTask(int _max); //устанавливает максимальное количество секций на задание
    virtual void setAuthorizationData(int id_task, const QString &data_base64); //устанавливает логин/пароль в base64 для вэб-авторизации
    virtual void setUserAgent(const QString &_uagent); //устанавливает идентификационные данные пользовательского агента (например: Opera)
    virtual void setReferer(int id_task,const QString &uref); //устанавливает реферера для id_task
    virtual void setAttemptInterval(const int sec); //устанавливает интервал между повторными попытками скачать секцию/задание
    virtual long long int totalSize(int id_task) const; //возвращает общий размер задания id_task
    virtual long long int sizeOnSection(int id_task, int _sect_num) const; //возвращает общий размер секции _sect_num для задания id_task
    virtual long long int totalLoadedOnTask(int id_task) const; //возвращает объем закачанного для задания целиком
    virtual long long int totalLoadedOnSection(int id_task, int _sect_num) const; //возвращает объем скачанного в секции _sect_num для задания id_task
    virtual long long int totalDownSpeed() const; //общая скорость скачивания
    virtual long long int downSpeed(int id_task) const; //скорость скачивания задания id_task
    virtual int taskStatus(int id_task) const; //возвращает состояние задания id_task
    virtual int errorNo(int id_task) const; //возвращает код ошибки задания
    virtual int loadTaskFile(const QString &_path); //загружает метаданные задания из указанного файла и возвращает идентификатор задания (при неудаче - 0)
    virtual int countSectionTask(int id_task) const; //возвращает количество активных секций в задании
    virtual bool acceptRanges(int id_task) const; //возвращает true, если возможна докачка задания id_task, иначе false
    virtual QString taskFilePath(int id_task) const; //вохвращает полный путь к локальному файлу
    virtual QString errorString(int _err) const; //возвращает строку по заданному коду ошибки
    virtual QString statusString(int _stat) const; //возвращает строку статуса по заданному коду
    virtual void setProxy(int id_task, const QUrl &_proxy, LInterface::ProxyType _ptype); //устанавливает прокси

signals:
    void sheduleImpulse(); //сигнал генериться с интервалом шедулера

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

    Task* getTaskSender(QObject* _sender) const;

private:
    QHash<int, Task*> *task_list; //хэш заданий по ключам
    QHash<HttpSection*, int> *sections; //хэш ключей заданий по ссылкам на секции
    QList<int> *squeue;
    QList<HttpSection*> *del_queue;
    int maxTaskNum; //хранится максимальный номер существующего задания
    int maxErrors;
    int maxSections;
    int attempt_interval; //интервал повтора
    QString uAgent;
    qint64 speed;

    QMutex *t_mutex; //мютекс для списка заданий
    QMutex *q_mutex; //мютекс для очереди секций

    bool shedule_flag;
    bool fullsize_res; //признак выделения/не выделения места под весь файл

    QTranslator *translator;
};

#endif // HTTPLOADER_H
