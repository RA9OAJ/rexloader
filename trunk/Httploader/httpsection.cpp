#include "httpsection.h"

HttpSection::HttpSection(QObject *parent) : QObject(parent) /*:
    QThread(parent)*/
{
    clear();
    mutex = new QMutex();
    watcher = new QTime();
    proxytype = QNetworkProxy::NoProxy;
    proxyaddr.clear();
    proxy_auth.clear();
}

void HttpSection::clear()
{
    //if(isRunning())quit();
    last_buf_size = 0;
    real_speed = 0;
    offset_f = 0;
    totalload = 0;
    totalsize = 0;
    _errno = 0;
    pause_flag = false;

    start_s = 0;
    finish_s = 0;

    down_speed = 0;//1310720; // 10Mbi/s

    user_agent = "Mozilla/5.0 (linux-gnu)";
    mode = 0;
    header.clear();

}

HttpSection::~HttpSection()
{

    delete(mutex);
    delete(watcher);
}

void HttpSection::run()
{
    //stopFlag = false;
    fl = new QFile();
    GTcpSocket *s = new GTcpSocket();
    soc = s;

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
    s->setDownSpeed(down_speed);
    connect(s,SIGNAL(connected()),this,SLOT(sendHeader())/*,Qt::DirectConnection*/);
    connect(s,SIGNAL(readyToRead()),this,SLOT(dataAnalising())/*,Qt::DirectConnection*/);
    connect(this,SIGNAL(beginTransfer()),s,SLOT(transferAct())/*,Qt::QueuedConnection*/); //переключаем внутренний шедулер сокета на внешний таймер
    connect(this,SIGNAL(setSpd(qint64)),s,SLOT(setDownSpeed(qint64))/*,Qt::QueuedConnection*/);
    connect(s,SIGNAL(error(QAbstractSocket::SocketError)), this,SLOT(socketErrorSlot(QAbstractSocket::SocketError))/*, Qt::DirectConnection*/); //обрабатываем ошибки сокета
    watcher->start();

    if(url.scheme().toLower() == "http") s->connectToHost(url.encodedHost(), (url.port() == -1 ? 80:url.port()), QTcpSocket::ReadWrite); //устанавливаем соединение
    else s->connectToHostEncrypted(url.encodedHost(), (url.port() == -1 ? 443:url.port()), QTcpSocket::ReadWrite);
    //exec();
}

void HttpSection::setFileName(const QString &filenm, int offset)
{
    flname = filenm;
    if(offset < 0)return;
    offset_f = offset;
}

void HttpSection::setOffset(qint64 offset)
{
    if(offset < 0)return;
    totalload = offset;
}

void HttpSection::setSection(qint64 start, qint64 finish)
{
    if(start < 0)start_s = 0;
    else start_s = start;
    if(finish < 0) finish_s = 0;
    else finish_s = finish;
    totalload = 0; //сбрасываем счетчик скачанных байт в секции, ибо границы секции были переопределены!!!
}

qint64 HttpSection::totalLoadOnSection() const
{
    return totalload;
}

int HttpSection::errorNumber() const
{
    return _errno;
}

void HttpSection::setUrlToDownload(const QString &url_target)
{
    url = QUrl::fromEncoded(url_target.toAscii());
}

void HttpSection::setUserAgent(const QString &uagent)
{
    if(uagent.isEmpty())return;
    user_agent = uagent;
}

void HttpSection::setReferer(const QString &uref)
{
    referer = uref;
}

qint64 HttpSection::totalFileSize() const
{
    return totalsize;
}

qint64 HttpSection::startByte() const
{
    return start_s;
}

qint64 HttpSection::finishByte() const
{
    return finish_s;
}

bool HttpSection::pauseState()
{
    return pause_flag;
}

void HttpSection::setAuthorizationData(const QString &data_base64)
{
    authorization = data_base64;
}

//--------SLOTS---------------------

void HttpSection::transferActSlot()
{
    if(!pause_flag)emit beginTransfer();
}

void HttpSection::startDownloading()
{
    //if(isRunning())return;
    //if(soc)return;
    mode = 0;
   _errno = 0;
   header.clear();

   run();
   //start();
}

void HttpSection::stopDownloading()
{
    if(!soc)return;
    //mutex->lock();
    //pauseDownloading(true);
    //------------28.01.2011-----
    if(soc->isOpen())
    {
        soc->close();

        if(soc->isOpen())soc->waitForDisconnected();
        qint64 lastSize = totalload;
        while(soc->bytesAvailableOnNetwork() && (mode == 0 || mode == 2))
        {
            if(!soc->getShedulerStatus())continue;
            dataAnalising();
            if(lastSize == totalload)break;
            lastSize = totalload;
        }
    }
    //pauseDownloading(false);
    //mutex->unlock();
    //---------------------------
    if(fl->isOpen())fl->close();
    //stopFlag = true;
    fl->deleteLater();
    soc->deleteLater();
    soc = 0;
    //if(isRunning())quit();
}

void HttpSection::pauseDownloading(bool pause)
{
    if(!pause)
    {
        setDownSpeed(down_speed);
        pause_flag = false;
        return;
    }
    emit setSpd(1024);
    pause_flag = true;
}

void HttpSection::sendHeader()
{
    if(!soc)return;
    QString target = (proxytype != QNetworkProxy::NoProxy) ? url.toEncoded() : url.encodedPath();
    if(!url.encodedQuery().isEmpty()) target += "?" + url.encodedQuery();
    QString _header = QString("GET %1 HTTP/1.1\r\nHost: %2\r\nAccept: */*\r\nUser-Agent: %3\r\n").arg(target,url.host(),user_agent);

    if(start_s > finish_s && finish_s != 0){qint64 _tmp = finish_s; finish_s = start_s; start_s = _tmp;}

    if(start_s != 0 || finish_s != 0 || totalload != 0)
    {
        _header += QString("Range: bytes=%1-%2").arg(QString::number(start_s+totalload), (finish_s == 0 ? "":QString::number(finish_s)));
        _header += "\r\n";
    }
    if(!authorization.isEmpty())
        _header += QString("Authorization: Basic %1\r\n").arg(authorization);

    _header += QString("Referer: http://%1/\r\n").arg(referer == "" ? url.host():referer);
    _header += QString("Connection: Keep-Alive\r\n\r\n");
    soc->write(_header.toAscii().data());
}

void HttpSection::dataAnalising()
{
    //if(stopFlag)return;
    if(!soc)return;
    if(watcher->elapsed() >= 1000)
    {
        real_speed = last_buf_size/watcher->elapsed()*1000; //для анализа реальной скорости
        last_buf_size = 0;
        watcher->start();
    }
    if(watcher->isNull())watcher->start();

    if(mode == 0)
    {
        while(soc->canReadLine())
        {
            QString cur_str = soc->readLine(1024);
            last_buf_size += cur_str.toAscii().length();

            if(cur_str.indexOf("HTTP/") == 0) {header["HTTP"] = cur_str.split(" ").value(1); continue;}
            if(cur_str.indexOf("\r\n") == 0 || cur_str.indexOf(0x0A) == 0) {mode = 1; break;}

            QStringList _tmp = cur_str.split(": ");
            header[_tmp.value(0).toLower()] = _tmp.value(1);

            if(cur_str.at(cur_str.size()-2) != 0x0D) header[_tmp.value(0).toLower()].chop(1);
            else header[_tmp.value(0).toLower()].chop(2);
        }
        if(_errno == QAbstractSocket::RemoteHostClosedError && mode != 1){_errno = HttpSection::SERV_CONNECT_ERROR;emit errorSignal(_errno); stopDownloading(); return;}
    }
    if(mode == 1)
    {
        int reqid = header["HTTP"].toInt();
        _errno = reqid;

        //--Определяем имя файла---
        QFileInfo flinfo(flname);
        if(flinfo.isDir())
        {
            if(flname[flname.size()-1]!='/')flname += "/";
            QString _tmpname = attachedFileName(header["content-disposition"]);

            if(_tmpname.isEmpty())
            {
                flinfo.setFile(url.toString());
                _tmpname = flinfo.fileName();
            }

            if(_tmpname.isEmpty())_tmpname = "noname.html";
            flname += _tmpname + QString(".%1.rldr").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
        }

        //---------------------------
        switch(reqid)
        {
        case 200:
            emit acceptQuery();

            totalsize = header["content-length"].toLongLong();
            emit totalSize(totalsize);
            emit fileType(header["content-type"]);
            if(header.contains("accept-ranges")) emit acceptRanges();
            if(lastmodified.isNull() && header.contains("last-modified"))
            {
                QLocale locale(QLocale::C);
                lastmodified = locale.toDateTime(header["last-modified"], "ddd, dd MMM yyyy hh:mm:ss 'GMT'");
            }
            break;
        case 206:
            emit acceptQuery();

            if(totalsize != 0 && totalsize != header["content-range"].split("/").value(1).toLongLong())
            {
               _errno = -2; //несоответствие размеров
                emit errorSignal(_errno);

                stopDownloading();
                return;
            }
            totalsize = header["content-range"].split("/").value(1).toLongLong();
            emit totalSize(totalsize);
            emit fileType(header["content-type"]);
            if(header.contains("accept-ranges") || header.contains("content-range")) emit acceptRanges();

            if(lastmodified.isNull() && header.contains("last-modified"))
            {
                QLocale locale(QLocale::C);
                lastmodified = locale.toDateTime(header["last-modified"], "ddd, dd MMM yyyy hh:mm:ss 'GMT'");
            }
            if(!lastmodified.isNull() && header.contains("last-modified"))
            {
                QLocale locale(QLocale::C);
                QDateTime _dtime = locale.toDateTime(header["last-modified"], "ddd, dd MMM yyyy hh:mm:ss 'GMT'");
                if(lastmodified != _dtime)
                {
                   _errno = -3; //несоответствие дат
                    emit mismatchOfDates(lastmodified, _dtime);
                    stopDownloading();
                    return;
                }
            }
            break;
        case 301:
        case 302:
        case 303:
        case 307:
            stopDownloading();
            if(header.contains("location"))
            {
                QUrl toUrl(header.value("location"));
                if(toUrl.scheme().isEmpty())
                {
                    if(header["location"].at(0)=='/')header["location"].remove(0,1);
                    QString location = url.scheme()+"://"+url.host();
                    if(url.port() != -1)location += ":" + url.port();
                    location += "/" + header["location"];
                    emit redirectToUrl(location);
                    return;
                }
                emit redirectToUrl(header["location"]);
            }
            else emit unidentifiedServerRequest();
            return;
        case 401:
        case 403:
        case 404:
        case 405:
        case 416:
        case 503:
            stopDownloading();
            emit errorSignal(_errno);
            return;
            break;

        default: emit stopDownloading(); unidentifiedServerRequest(); return; break;
        }
        if(!fl->isOpen())
        {
            fl->setFileName(flname);
            if(fl->exists())fl->open(QFile::ReadWrite);
            else fl->open(QFile::WriteOnly);
            fl->seek(offset_f+start_s+totalload);
        }
        if(mode != 0) mode = 2;
        else return;

    }
    if(mode == 2)
    {
        qint64 cur_bloc = fl->write(soc->readAll());
        if(cur_bloc == -1)
        {
            _errno = -4; //ошибка при записи в файл
            stopDownloading();
            emit errorSignal(_errno);
            return;
        }
        last_buf_size += cur_bloc;
        totalload += cur_bloc;
        emit transferCompleted(cur_bloc);

        qint64 targetsize = finish_s ? finish_s - start_s + 1 : totalsize - start_s;
        if((soc->state() != QTcpSocket::ConnectedState && soc->bytesAvailable() == 0 && soc->bytesAvailableOnNetwork() == 0) || totalload == targetsize)
        {
            fl->close();
            stopDownloading();
            emit downloadingCompleted();
            return;
        }
    }
}

void HttpSection::socketErrorSlot(QAbstractSocket::SocketError _err)
{
    _errno = _err;
    if(_err == QAbstractSocket::RemoteHostClosedError)return;
    stopDownloading();

    emit errorSignal(_errno);
}

void HttpSection::setDownSpeed(qint64 spd)
{
    down_speed = spd;
    emit setSpd(spd);
}

QDateTime HttpSection::lastModified() const
{
    return lastmodified;
}

void HttpSection::setLastModified(const QDateTime &_dtime)
{
    if(!_dtime.isValid())return;
    lastmodified = _dtime;
}

qint64 HttpSection::downSpeed() const
{
    return down_speed;
}

qint64 HttpSection::realSpeed() const
{
    return real_speed;
}

bool HttpSection::freedMemory() const
{
    return (!fl && !soc);
}

QString HttpSection::fileName() const
{
    return flname;
}

QString HttpSection::attachedFileName(const QString &cont_dispos) const
{
    if(cont_dispos.indexOf("filename") < 0) return QString();
    QStringList words = cont_dispos.split("; ");
    for(int i = 0; i<words.size(); ++i)
    {
        if(words.value(i).indexOf("filename")<0)continue;
        QString str = words.value(i).split("=\"").value(1);
        str.chop(1);
        return str;
    }
    return QString();
}

void HttpSection::setProxy(const QUrl &_proxy, QNetworkProxy::ProxyType _ptype, const QString &base64_userdata)
{
    if(_ptype == QNetworkProxy::NoProxy)
    {
        proxyaddr.clear();
        proxytype = _ptype;
        proxy_auth.clear();
        return;
    }

    proxyaddr = _proxy;
    proxytype = _ptype;
    proxy_auth = base64_userdata;
}
