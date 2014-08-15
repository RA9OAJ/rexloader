#include "ftpsection.h"

qint16 FtpSection::_start_port = 0;
qint16 FtpSection::_end_port = 0;

FtpSection::FtpSection(QObject *parent) :
    QObject(parent)
{
    clear();
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
    if(end_port >= start_port)
    {
        _start_port = start_port;
        _end_port = end_port;
    }
    else
    {
        _start_port = end_port;
        _end_port = start_port;
    }
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
    authdata = data_base64;
}

void FtpSection::clear()
{
    start_flag = false;
    ftp_mode = FtpPassive;
    proxytype = QNetworkProxy::NoProxy;
    start_byte = end_byte = 0;
    _errnum = -1;
    ftp_stack.clear();
    authdata = "anonymous\r\nanonymous";
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
    if(start_flag)
        return;

    start_flag = true;
    run();
}

void FtpSection::stopDownloading()
{
   start_flag = false;
   if(msoc && msoc->state() == QTcpSocket::ConnectedState)
   {
       if(!soc)
           sendCommand("QUIT");
       else sendCommand("ABOR");
   }
}

void FtpSection::setDownSpeed(qint64 spd)
{
    down_speed = spd;
}


void FtpSection::run()
{
    fl = new QFile();
    QTcpSocket *s = new QTcpSocket();
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

    connect(s,SIGNAL(readyRead()),this,SLOT(dataAnalising()));
    connect(s,SIGNAL(error(QAbstractSocket::SocketError)), this,SLOT(socketErrorSlot(QAbstractSocket::SocketError))); //обрабатываем ошибки сокета

    int port = (url.port() == -1 ? 21:url.port());
    s->connectToHost(url.encodedHost(), port, QTcpSocket::ReadWrite); //устанавливаем соединение
    ftp_stack.append(Pair("CONNECT",0));

    emit sectionMessage(LInterface::MT_INFO,tr("Попытка соединения с %1 на порту %2").arg(url.host(),QString::number(port)),QString());
}

QString FtpSection::nextCommand() const
{
    return cmd_stack.first();
}

QString FtpSection::takeNextCommand()
{
    return cmd_stack.takeFirst();
}

IPAddress FtpSection::getAddress(const QString &str) const
{
    IPAddress addr;
    if(str.indexOf(",") > -1)
    {
        QString src = str;
        src = src.replace(QRegExp("^.+\\(+"),"");
        src = src.replace(QRegExp("\\)\\..+"),"");
        QString srcp = src;
        addr.first = src.replace(QRegExp("(,\\d{1,3}){2}$"),"").replace(",",".");
        srcp.replace(QRegExp("^(\\d{1,3},){4}"),"");
        addr.second = (srcp.split(",").value(0).toInt()*256) + srcp.split(",").value(1).toInt();
    }
    else if(str.indexOf(".") > -1)
    {

    }
    return addr;
}

void FtpSection::sendCommand(const QString &cmd)
{
    ftp_stack.append(Pair(cmd.split(" ").value(0),0));
    msoc->write(QString(cmd+"\r\n").toAscii());
    //msoc->flush();

    QString msg = cmd;
    if(cmd.indexOf("PASS ") == 0)
        msg = msg.replace(cmd.split(" ").value(1),QString("******"));
    emit sectionMessage(LInterface::MT_IN,msg,QString());
}

void FtpSection::sendNextCommand()
{
    sendCommand(takeNextCommand());
}

void FtpSection::appendNextCommand(const QString &cmd)
{
    cmd_stack.append(cmd);
}

void FtpSection::dataAnalising()
{
    if(ftp_stack.last().first == "CONNECT")
        emit sectionMessage(LInterface::MT_INFO,tr("Соединение с %1 установлено на порту %2").arg(url.host(),QString::number(msoc->peerPort())),QString());

    QString ansvr,msg;
    while(msoc->canReadLine() || msoc->waitForReadyRead(10))
    {
        QString str = msoc->readLine(4096);
        QString spltr = str.indexOf("-") == 3 ? "-" : " ";

        int ftp_code = str.split(spltr).value(0).toInt();
        if(ftp_code)
        {
            ftp_stack.last().second = ftp_code;
            if(!ansvr.isEmpty())
                msg += ansvr;

            ansvr = str;
        }
        else msg += str;
    }
    emit sectionMessage(LInterface::MT_IN,ansvr,msg);

    switch (ftp_stack.last().second) {
    case 220:
        //указываем логин логина
        sendCommand(QString("USER %1").arg(authdata.split("\r\n").value(0)));
        break;
    case 331:
        //указываем пароля
        sendCommand(QString("PASS %1").arg(authdata.split("\r\n").value(1)));
        break;
    case 230:
        //логин совершен
        sendCommand("HELP");
        break;
    case 214:
        //ответ на запрос HELP
        if(msg.indexOf("REST") != -1)
            emit acceptRanges();
        sendCommand("TYPE I");
        appendNextCommand(QString("LIST %1").arg(url.path().isEmpty() ? "/" : url.path()));
        break;
    case 200: //ответ на TYPE
        if(ftp_mode == FtpPassive)
            sendCommand("PASV");
        else
        {
            //тут идет подготовка к открытию и прослушиванию порта из пула портов
            //sendCommand("PORT %1,%2,%3,%4,%5,%6").arg();
        }
        break;
    case 227:
        //тут создае сокет данных и подключаемся по указанному сервером адресу

        //sendNextCommand();
        break;
    case 530:
        //некорректный логин
        _errnum = AUTH_ERROR;
        stopDownloading();
        emit errorSignal(_errnum);
        break;
    case 421:
    case 221: //quit
        if(msoc->isOpen())
            msoc->close();

        while(msoc->isOpen())
            msoc->waitForDisconnected();

        msoc->deleteLater();
        msoc = 0;
        _start_port = false;
        break;
    default:
        break;
    }
}

void FtpSection::socketErrorSlot(QAbstractSocket::SocketError _err)
{
    if(_err == QTcpSocket::SocketTimeoutError || (ftp_stack.last().first == "QUIT" && _err == QTcpSocket::RemoteHostClosedError))
        return;

    stopDownloading();
    emit errorSignal(_err);
}
