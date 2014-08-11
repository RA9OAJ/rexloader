#include "ftpsection.h"

FtpSection::FtpSection(QObject *parent) :
    QObject(parent)
{
}

FtpSection::~FtpSection()
{
}

void FtpSection::setFilename(const QString &file)
{
    filename = file;
}

void FtpSection::setUrl(const QString &url_target)
{
    url = QUrl::fromEncoded(url_target.toAscii());
}

void FtpSection::setSection(qint64 start_b, qint64 end_b)
{
    if(start_b < 0)start_byte = 0;
    else start_byte = start_b;
    if(end_b < 0) end_byte = 0;
    else end_byte = end_b;
    total_load = 0; //сбрасываем счетчик скачанных байт в секции, ибо границы секции были переопределены!!!

    emit sectionMessage(LInterface::MT_INFO,tr("Установлены границы секции с %1 байта по %2 байт").arg(QString::number(start_b), QString::number(end_b)),QString());
}

void FtpSection::setOffset(qint64 offset)
{
    if(offset < 0)return;
    total_load = offset;

    emit sectionMessage(LInterface::MT_INFO,tr("Установлено смещение в секции на %1").arg(QString::number(offset)),QString());
}

void FtpSection::setFtpMode(FtpSection::FtpMode mode)
{
    ftp_mode = mode;
}

void FtpSection::setPortPool(qint16 start_port, qint16 end_port)
{
}

void FtpSection::setProxyType(QNetworkProxy::ProxyType ptype)
{
}

void FtpSection::setProxy(const QUrl &_proxy, QNetworkProxy::ProxyType _ptype, const QString &base64_userdata)
{
}

void FtpSection::setLastModified(const QDateTime &_dtime)
{
}

qint64 FtpSection::totalLoadOnSection() const
{
    return total_load;
}

qint64 FtpSection::totalFileSize() const
{
    return total_filesize;
}

qint64 FtpSection::startByte() const
{
    return start_byte;
}

qint64 FtpSection::finishByte() const
{
    return end_byte;
}

qint64 FtpSection::downSpeed() const
{
    return down_speed;
}

QString FtpSection::fileName() const
{
    return filename;
}

int FtpSection::errorNumber() const
{
    return _errnum;
}

void FtpSection::setAuthorizationData(const QString &data_base64)
{
}

void FtpSection::clear()
{
}

int FtpSection::socketError() const
{
    if(msoc && msoc->error())
        return msoc->error();
    else if(soc)
        return soc->error();

    return 0;
}

qint64 FtpSection::realSpeed() const
{
    return real_speed;
}

QDateTime FtpSection::lastModified() const
{
    return QDateTime();
}

void FtpSection::transferActSlot()
{
}

void FtpSection::startDownloading()
{
    run();
}

void FtpSection::stopDownloading()
{
}

void FtpSection::setDownSpeed(qint64 spd)
{
    down_speed = spd;
}


void FtpSection::run()
{
    fl = new QFile();
    GTcpSocket *s = new GTcpSocket();
    msoc = s;

    if(proxytype != QNetworkProxy::NoProxy)
    {
        myproxy = new QNetworkProxy;
        myproxy->setHostName(proxyaddr.host());
        myproxy->setPort(proxyaddr.port());
        myproxy->setType(proxytype);
        if(proxy_auth != "")
        {
            QString udata(QByteArray::fromBase64(proxy_auth.toAscii()));
            QStringList _udata = udata.split(":");
            if(_udata.size() > 1)
            {
                myproxy->setUser(_udata.value(0));
                myproxy->setPassword(_udata.value(1));
            }
        }
        s->setProxy(*myproxy);
    }


    s->setMode(false); //переключаем режим сокета на внешний тайминг
    //s->setDownSpeed(spd);
    //connect(s,SIGNAL(connected()),this,SLOT(sendHeader()));
    connect(s,SIGNAL(readyToRead()),this,SLOT(dataAnalising()));
    connect(this,SIGNAL(beginTransfer()),s,SLOT(transferAct())); //переключаем внутренний шедулер сокета на внешний таймер
    connect(this,SIGNAL(setSpd(qint64)),s,SLOT(setDownSpeed(qint64)));
    connect(s,SIGNAL(error(QAbstractSocket::SocketError)), this,SLOT(socketErrorSlot(QAbstractSocket::SocketError))); //обрабатываем ошибки сокета

    int port = (url.port() == -1 ? 21:url.port());
    s->connectToHost(url.encodedHost(), port, QTcpSocket::ReadWrite); //устанавливаем соединение

    emit sectionMessage(LInterface::MT_INFO,tr("Попытка соединения с %1 на порту %2").arg(url.host(),QString::number(port)),QString());
}

void FtpSection::sendHeader()
{

}

void FtpSection::dataAnalising()
{
    if(msoc->canReadLine())
    {
        QString str = msoc->readLine(4096);
        int ftp_code = str.split(" ").value(0).toInt();
        emit sectionMessage(LInterface::MT_IN,str,QString());
    }
}

void FtpSection::socketErrorSlot(QAbstractSocket::SocketError _err)
{

}
