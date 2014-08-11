#ifndef FTPSECTION_H
#define FTPSECTION_H

#include <QObject>
#include <QDateTime>
#include <QHash>
#include <QPointer>
#include <QByteArray>
#include <QUrl>
#include <QFile>
#include <QList>
#include <QTime>
#include <QStringList>
#include <QNetworkProxy>

#include "gtcpsocket.h"
#include "../LoaderInterface.h"

class FtpSection : public QObject
{
    Q_OBJECT
public:
    explicit FtpSection(QObject *parent = 0);
    virtual ~FtpSection();

    enum Errors{
        SIZE_ERROR = -2, //Изменился размер файла на сервере | критичная
        DATE_ERROR = -3, //Изменилась дата модификации на сервере | критичная
        WRITE_ERROR = -4, //Ошибка записи в файл | критичная
        SERV_CONNECT_ERROR = -5, //Сервер разорвал соединение | не критичная
        FILE_NOT_AVAILABLE = -6 //файл больше недоступен по данной ссылке
    };

    enum FtpMode{
        FtpPassive,
        FtpActive
    };

    void setFilename(const QString &file);
    void setUrl(const QString &url_target);
    void setSection(qint64 start_b, qint64 end_b);
    void setOffset(qint64 offset);
    void setFtpMode(FtpMode mode);
    void setPortPool(qint16 start_port, qint16 end_port); //задает интервал портов при активном режиме
    void setProxyType(QNetworkProxy::ProxyType ptype);
    void setProxy(const QUrl &_proxy, QNetworkProxy::ProxyType _ptype, const QString &base64_userdata);
    void setLastModified(const QDateTime &_dtime); //устанавливает дату последнего изменения файла
    qint64 totalLoadOnSection() const; //возвращает итоговый размер скачаных данных в секции
    qint64 totalFileSize() const; //возвращает полный размер скачиваемого файла
    qint64 startByte() const; //возвращает номер начального байта секции
    qint64 finishByte() const; //возвращает номер последнего байта секции
    qint64 downSpeed() const; //возвращает максимально разрешенную скорость
    QString fileName() const; //возвращает локальный путь до файла-назначения
    int errorNumber() const; // возвращает номер ошибки секции
    void setAuthorizationData(const QString &data_base64); //устанавливает логин/пароль в base64 для авторизации
    void clear(); //сбрасывает все настройки и данные секции;
    int socketError() const; //возвращает ошибки сокета
    qint64 realSpeed() const; //возвращает реальную возможную скорость скачивания
    QDateTime lastModified() const; //возвращает дату последней модификации файла

public slots:
    void transferActSlot(); //слот-посредник
    void startDownloading(); //старт закачки
    void stopDownloading(); //останов закачки
    void setDownSpeed(qint64 spd);//устанавливает скорость скачиваниея

signals:
    void downloadingCompleted(); //скачивание секции завершено (т.е. данные буферов записаны в файл, однако надо проверить на полноту выполнения задания)
    void totalSize(qint64 sz); //сигнал генерится при выяснении размера файла скачивания
    void setSpd(qint64 spd); //сигнал передает предельную скорость скачивания
    void acceptRanges(); //сигнал генерится, если сервер предположил возможность докачки
    void rangeNotAccepted(); //сигнал указывает на то, что докачка не возможна
    void fileType(QString); //сигнал генерится, когда определяем MIME тип файла
    void acceptQuery(); //сигнал генерится при удовлетворяющем ответе от сервера на запрос закачки файла
    void unidentifiedServerRequest(); //неопределенный ответ от сервера
    void mismatchOfDates(QDateTime localfile, QDateTime remotefile); //генерится в случае различий дат модификации локального и скачиваемого файлов
    void errorSignal(int); // генерится в случаях критических ошибок при попытке скачивания
    void transferCompleted(qint64); //генерится при удачной записи скачанных данных из буфера в файл
    void beginTransfer(); //сигнал-посредник
    void sectionMessage(int ms_type,const QString &message, const QString &more); //сигнал сообщения о событии от секции
    void isDir(const QUrl &url, const QStringList &dir_list); //сигнал сообщает о том, что по указанному URL находится каталог и возвращает его листинг

protected slots:
    void run();

protected slots:
    void sendHeader(); // слот посылает запрос на получение содержимого по URL
    void dataAnalising(); // слот вызывается при наличии во входном буфере сокета данных и анализирует их, сдесь же происходит и запись в файл
    void socketErrorSlot(QAbstractSocket::SocketError _err);//слот для обработки ошибок сокета

private:
    QPointer<QFile> fl; //файл для записи данных
    QPointer<GTcpSocket> msoc; //управляющий сокет
    QPointer<GTcpSocket> soc; //сокет данных
    QString filename; //путь до локального файла
    QUrl url; //URL файла
    FtpMode ftp_mode; //режим работы FTP клиента
    int _errnum; //номер внутренней ошибки

    QByteArray mbuf; //буфер для управляющего сокета
    QByteArray inbuf; //буфер для входящих данных

    qint64 start_byte; //начальный байт секции
    qint64 end_byte; //последний байт секции
    qint64 offset_byte; //смещение в секции
    qint64 total_load; //скачано в секции
    qint64 total_filesize; //общий объём файла
    qint64 down_speed; //предельная скорость загрузки
    qint64 real_speed; //реальная скорость загрузки

    QString authdata; //данные для аутентификации в base64
    QUrl proxyaddr; //адрес прокси сервера
    QNetworkProxy::ProxyType proxytype; //тип прокси сервера
    QString proxy_auth; //данны для аутентификации на прокси сервере
    QNetworkProxy *myproxy; //прокси

    QHash<qint16,int> port_pool;
    qint16 start_port;
    qint16 end_port;

};

#endif // FTPSECTION_H
