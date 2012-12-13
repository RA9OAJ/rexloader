/*
Project: REXLoader (Downloader, plugin: HttpLoader), Source file: httpsection.h
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

#ifndef HTTPSECTION_H
#define HTTPSECTION_H

#include <QThread>
#include <QTime>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QStringList>
#include <QDateTime>
#include <QMutex>
#include <QLocale>
#include <QPointer>
#include <QNetworkProxy>
#include "../LoaderInterface.h"
#include "zlib.h"
#include "gtcpsocket.h"

class HttpSection : public QObject/*: public QThread*/
{
    Q_OBJECT

public:
    explicit HttpSection(QObject *parent = 0);
    virtual ~HttpSection();

    enum Errors{
        SIZE_ERROR = -2, //Изменился размер файла на сервере | критичная
        DATE_ERROR = -3, //Изменилась дата модификации на сервере | критичная
        WRITE_ERROR = -4, //Ошибка записи в файл | критичная
        SERV_CONNECT_ERROR = -5, //Сервер разорвал соединение | не критичная
        FILE_NOT_AVAILABLE = -6 //файл больше недоступен по данной ссылке
    };

    void setFileName(const QString &filenm, int offset=0); // имя файла-назначения и смещение по отношению к началу (нужно учитывать размер метаданных)
    void setSection(qint64 start=0, qint64 finish=0); //устанавливает границы загружаемой секции
    void setOffset(qint64 offset=0); //устанавливает смещение относительно start в случае докачки секции не с начала...
    void setUrlToDownload(const QString &url_target); // устанавливает URL цели для скачивания
    void setUserAgent(const QString &uagent); //устанавливаем клиентский агент
    void setReferer(const QString &uref); //устанавливаем реферера на ресурс
    void setLastModified(const QDateTime &_dtime); //устанавливает дату последнего изменения файла
    qint64 totalLoadOnSection() const; //возвращает итоговый размер скачаных данных в секции
    qint64 totalFileSize() const; //возвращает полный размер скачиваемого файла
    qint64 startByte() const; //возвращает номер начального байта секции
    qint64 finishByte() const; //возвращает номер последнего байта секции
    qint64 downSpeed() const; //возвращает максимально разрешенную скорость
    QString fileName() const; //возвращает локальный путь до файла-назначения
    QDateTime lastModified() const; //возвращает дату последней модификации файла
    int errorNumber() const; // возвращает номер ошибки секции
    bool pauseState();// возвращает статус флага паузы
    void setAuthorizationData(const QString &data_base64); //устанавливает логин/пароль в base64 для вэб-авторизации
    void clear(); //сбрасывает все настройки и данные секции;
    int socketError() const; //возвращает ошибки сокета
    qint64 realSpeed() const; //возвращает реальную возможную скорость скачивания
    void setCookie(const QString &cookie);
    QString getCookie() const;
    bool freedMemory()const;
    void setProxy(const QUrl &_proxy, QNetworkProxy::ProxyType _ptype, const QString &base64_userdata);

public slots:
    void transferActSlot(); //слот-посредник
    void startDownloading(); //старт закачки
    void stopDownloading(); //останов закачки
    void setDownSpeed(qint64 spd);//устанавливает скорость скачиваниея
    void pauseDownloading(bool pause); //приостанавливает процесс закачки, не разрывая при этом соединения

signals:
    void downloadingCompleted(); //скачивание секции завершено (т.е. данные буферов записаны в файл, однако надо проверить на полноту выполнения задания)
    void redirectToUrl(QString); //сигнал генерится при переадресации сервером на другой URL
    void totalSize(qint64 sz); //сигнал генерится при выяснении размера файла скачивания
    void setSpd(qint64 spd); //сигнал передает предельную скорость скачивания
    void acceptRanges(); //сигнал генерится, если сервер поддерживает докачку файлов
    void fileType(QString); //сигнал генерится, когда определяем MIME тип файла
    void acceptQuery(); //сигнал генерится при удовлетворяющем ответе от сервера на запрос закачки файла
    void unidentifiedServerRequest(); //неопределенный ответ от сервера
    void mismatchOfDates(QDateTime localfile, QDateTime remotefile); //генерится в случае различий дат модификации локального и скачиваемого файлов
    void errorSignal(int); // генерится в случаях критических ошибок при попытке скачивания
    void transferCompleted(qint64); //генерится при удачной записи скачанных данных в файл
    void beginTransfer(); //сигнал-посредник
    void sectionMessage(int ms_type,const QString &message, const QString &more); //сигнал сообщения о событии от секции

protected:
    void run();
    QString attachedFileName(const QString &cont_dispos) const; //возвращает имя вложенного файла, если сервер организовал вложение, иначе пустую QString

protected slots:
    void sendHeader(); // слот посылает запрос на получение содержимого по URL
    void dataAnalising(); // слот вызывается при наличии во входном буфере сокета данных и анализирует их, сдесь же происходит и запись в файл
    void socketErrorSlot(QAbstractSocket::SocketError _err);//слот для обработки ошибок сокета
    QByteArray ungzipData(QByteArray &data); //распаковывает gzip упакованные секции при получении сжатых данный от сервера

private:
    QPointer<GTcpSocket> soc; // указатель на сокет (нужен ли он?)
    QUrl url;
    QString flname; //имя(путь) файла закачки
    int offset_f; //смещение в файле
    qint64 totalload; //размер скачанного фрагмента
    qint64 totalsize; //общий размер файла
    int _errno; //номер ошибки
    int mode; //режим анализа/скачивания
    bool pause_flag; //отражает текущее состояние секции закачка/пауза без разрыва соединения
    //bool authorize_flag; //указывает на необходимость авторизации

    qint64 start_s; //начальный байт секции закачки
    qint64 finish_s; //конечный байт секции закачки, еслт он ==0 то закачиваем до конца
    qint64 real_speed;
    qint64 last_buf_size;
    QDateTime lastmodified; //дата прошлой модификации удаленного файла для скачивания

    qint64 down_speed; //предельная скорость скачивания
    qint64 chunked_size; //общий размер текущей секции
    qint64 chunked_load;
    qint64 decompressSize; //текущий размер скачанного в данной секции
    QByteArray inbuf; //буфер для нераспакованных данных gzip

    QString user_agent; //для протокола HTTP идентификация клиента
    QString referer; //реферер для HTTP
    QString authorization; //логин/пароль в base64 для вэб-авторизации
    QPointer<QFile> fl; //файл для записи данных
    QHash<QString, QString> header; //хеш-массив для хранения ответа сервера
    QTime *watcher;
    QMutex *mutex;

    QUrl proxyaddr; //адрес прокси сервера
    QNetworkProxy::ProxyType proxytype; //тип прокси сервера
    QString proxy_auth; //данны для аутентификации на прокси сервере
    QNetworkProxy *myproxy;

    QString cookie_string;
};

#endif // HTTPSECTION_H
