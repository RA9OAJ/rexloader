/*
Copyright (C) 2010-2013  Sarvaritdinov R.

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
    chunked_size = -1;
    chunked_load = 0;
    decompressSize = 0;
    inbuf.clear();

    emit sectionMessage(LInterface::MT_INFO,tr("Создана секция"),QString());
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

    int port = 80;
    if(url.scheme().toLower() == "http")
    {
        port = (url.port() == -1 ? 80:url.port());
        s->connectToHost(url.encodedHost(), port, QTcpSocket::ReadWrite); //устанавливаем соединение
    }
    else
    {
        port = (url.port() == -1 ? 443:url.port());
        s->setPeerVerifyMode(QSslSocket::VerifyNone);
        s->connectToHostEncrypted(url.encodedHost(), port, QTcpSocket::ReadWrite);
    }
    //exec();
    emit sectionMessage(LInterface::MT_INFO,tr("Попытка соединения с %1 на порту %2").arg(url.host(),QString::number(port)),QString());
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

    emit sectionMessage(LInterface::MT_INFO,tr("Установлено смещение в секции на %1").arg(QString::number(offset)),QString());
}

void HttpSection::setSection(qint64 start, qint64 finish)
{
    if(start < 0)start_s = 0;
    else start_s = start;
    if(finish < 0) finish_s = 0;
    else finish_s = finish;
    totalload = 0; //сбрасываем счетчик скачанных байт в секции, ибо границы секции были переопределены!!!

    emit sectionMessage(LInterface::MT_INFO,tr("Установлены границы секции с %1 байта по %2 байт").arg(QString::number(start_s), QString::number(finish_s)),QString());
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
    if(watcher->elapsed() >= 1000)
    {
        real_speed = last_buf_size/watcher->elapsed()*1000; //для анализа реальной скорости
        last_buf_size = 0;
        watcher->start();
    }

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
    emit sectionMessage(LInterface::MT_INFO,tr("Секция остановлена"),QString());
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
    emit sectionMessage(LInterface::MT_INFO,tr("Соединение с узлом установлено"),QString());

    QString target = (proxytype != QNetworkProxy::NoProxy) ? url.toEncoded() : url.encodedPath();
    if(!url.encodedQuery().isEmpty()) target += "?" + url.encodedQuery();
    QString _header = QString("GET %1 HTTP/1.1\r\nHost: %2\r\nAccept: */*\r\nAccept-Encoding: gzip, deflate\r\nUser-Agent: %3\r\n").arg(target,url.host(),user_agent);

    if(start_s > finish_s && finish_s != 0){qint64 _tmp = finish_s; finish_s = start_s; start_s = _tmp;}

    if(start_s != 0 || finish_s != 0 || totalload != 0)
    {
        _header += QString("Range: bytes=%1-%2").arg(QString::number(start_s+totalload), (finish_s == 0 ? "":QString::number(finish_s)));
        _header += "\r\n";
        _header += "Pragma: no-cache\r\nCache-Control: no-cache\r\n";
        /*if(!_etag.isEmpty())
            _header += QString("If-Range: %1\r\n").arg(_etag);
        else if(!lastmodified.isNull())
        {
            QLocale locale(QLocale::C);
            _header += QString("If-Range: %1\r\n").arg(locale.toString(lastmodified,"ddd, dd MMM yyyy hh:mm:ss 'GMT'"));
        }*/
    }
    if(!authorization.isEmpty())
        _header += QString("Authorization: %1\r\n").arg(authorization);
    if(!referer.isEmpty())_header += QString("Referer: %1\r\n").arg(referer);
    if(!cookie_string.isEmpty())
        _header += QString("Cookie: %1\r\n").arg(cookie_string);
    _header += QString("Connection: Keep-Alive\r\n\r\n");
    soc->write(_header.toAscii().data());
    emit sectionMessage(LInterface::MT_OUT,tr("Отправка HTTP заголовка"),_header);
}

void HttpSection::dataAnalising()
{
    if(!soc)return;

    if(watcher->isNull())watcher->start();

    QString _request;
    if(mode == 0)
    {
        while(soc->canReadLine())
        {
            QString cur_str = soc->readLine(1024);
            _request += cur_str;
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
        emit sectionMessage(LInterface::MT_IN,tr("Получен ответ %1").arg(header["HTTP"]),_request);
        int reqid = header["HTTP"].toInt();
        _errno = reqid;

        //--Определяем имя файла---
        QFileInfo flinfo(flname);
        if(flinfo.isDir() || (header.contains("content-disposition") && !flinfo.exists()))
        {
            if(flname[flname.size()-1]!='/' && flinfo.isDir())flname += "/";
            if(flname[flname.size()-1]!='/' && header.contains("content-disposition")) flname = flinfo.absolutePath() + "/";
            QStringList _tmpname = attachedFileName(header["content-disposition"]);

            if(_tmpname.isEmpty())
            {
                flinfo.setFile(url.path());
                _tmpname.append(flinfo.fileName());
            }

            if(_tmpname.value(0).isEmpty())_tmpname.append("noname.html");

            if(_tmpname.value(0).indexOf(QRegExp("(%[0-9a-zA-Z]{2})")) > -1)
            {
                QTextCodec *codec = QTextCodec::codecForName(_tmpname.value(1).toAscii());
                if(codec)
                    _tmpname[0] = codec->toUnicode(QByteArray::fromPercentEncoding(_tmpname.value(0).toAscii()));
            }

            flname += _tmpname.value(0) + QString(".%1.rldr").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
            emit newFileName(flname);
        }

        //---------------------------
        switch(reqid)
        {
        case 200:
            if(header.contains("content-length"))
                totalsize = header["content-length"].toLongLong();
            emit totalSize(totalsize);
            emit fileType(header["content-type"]);

            if(lastmodified.isNull() && header.contains("last-modified"))
            {
                QLocale locale(QLocale::C);
                lastmodified = locale.toDateTime(header["last-modified"], "ddd, dd MMM yyyy hh:mm:ss 'GMT'");
            }

            if(start_s || finish_s)
            {
                emit sectionMessage(LInterface::MT_WARNING,tr("Докачка не поддерживается"),QString());
                emit rangeNotAccepted();
            }
            else emit acceptQuery();
            break;
        case 206:
            emit acceptQuery();

            if(totalsize != 0 && totalsize != header["content-range"].split("/").value(1).toLongLong())
            {
                _errno = SIZE_ERROR; //несоответствие размеров
                emit errorSignal(_errno);

                stopDownloading();
                return;
            }
            totalsize = header["content-range"].split("/").value(1).toLongLong();
            emit totalSize(totalsize);
            emit fileType(header["content-type"]);
            if(header.contains("accept-ranges") || header.contains("content-range"))
            {
                emit acceptRanges();
                emit sectionMessage(LInterface::MT_INFO,tr("Докачка поддерживается"),QString());
            }

            if(lastmodified.isNull() && header.contains("last-modified"))
            {
                QLocale locale(QLocale::C);
                lastmodified = locale.toDateTime(header["last-modified"], "ddd, dd MMM yyyy hh:mm:ss 'GMT'");
            }
            /*if(!lastmodified.isNull() && header.contains("last-modified"))
            {
                QLocale locale(QLocale::C);
                QDateTime _dtime = locale.toDateTime(header["last-modified"], "ddd, dd MMM yyyy hh:mm:ss 'GMT'");
                if(lastmodified != _dtime)
                {
                    _errno = DATE_ERROR; //несоответствие дат
                    emit mismatchOfDates(lastmodified, _dtime);
                    stopDownloading();
                    return;
                }
            }*/
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
        case 400:
        case 401:
        case 403:
        case 404:
        case 405:
        case 409:
        case 410:
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
            if(flname.right(5) != ".rldr")
            {
                flname += QString(".%1.rldr").arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
                emit newFileName(flname);
            }

            fl->setFileName(flname);
            if(fl->exists())fl->open(QFile::ReadWrite);
            else fl->open(QFile::WriteOnly);
            if(!fl->isOpen())
            {
                _errno = WRITE_ERROR;
                emit errorSignal(_errno);
                return;
            }
            fl->seek(offset_f+start_s+totalload);
        }
        if(mode != 0) mode = 2;
        else return;

    }
    if(mode == 2 && soc && fl)
    {
        qint64 cur_bloc = 0;

        if(header.contains("transfer-encoding"))
        {
            if(chunked_size < 0) //распознаем строку с размером секции
            {
                if(!soc->canReadLine()) return;
                QString buf = soc->readLine(1024);
                chunked_size = buf.toLongLong(0,16);
                chunked_load = 0;

                if(!chunked_size)
                {
                    decompressSize += fl->write(ungzipData(inbuf));
                    if(!decompressSize && inbuf.size() > 0 && header["transfer-encoding"].toLower().indexOf("deflate") < 0)
                        decompressSize += fl->write(inbuf); //если просто chunked без deflate сжатия

                    if(decompressSize < 0)
                    {
                        _errno = -4; //ошибка при записи в файл
                        stopDownloading();
                        emit errorSignal(_errno);
                        return;
                    }
                    fl->close();
                    stopDownloading();
                    emit downloadingCompleted();
                    decompressSize = 0;
                    inbuf.clear();
                    return;
                }
            }
            qint64 lbufs = inbuf.size();
            inbuf.append(soc->read(chunked_size - chunked_load));
            chunked_load += inbuf.size() - lbufs;
            cur_bloc = inbuf.size() - lbufs;
        }
        else if(header.contains("content-encoding"))
            inbuf.append(soc->readAll());
        else cur_bloc = fl->write(soc->readAll());

        if(chunked_size > 0 && chunked_size - chunked_load == 0) //если скачка по методу Chunked
        {
            chunked_size = -1; //если выкачали секцию целиком, то сбрасываем общий размер текущей chunked-секции в -1
            while(soc->read(1) != "\n");
        }
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
        if((soc->state() != QTcpSocket::ConnectedState && soc->bytesAvailable() == 0 && soc->bytesAvailableOnNetwork() == 0) || (totalload == targetsize && totalload != 0))
        {
            if(header.contains("content-encoding") && inbuf.size())
            {
                totalload = fl->write(ungzipData(inbuf));
                emit transferCompleted(totalload);
                inbuf.clear();
            }

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
    if(_err == QAbstractSocket::RemoteHostClosedError)
    {
        qint64 targetsize = finish_s ? finish_s - start_s + 1 : totalsize - start_s;
        qint64 canload = totalload + soc->bytesAvailableOnNetwork() + soc->bytesAvailable();
        if((canload == targetsize && totalsize) || (!totalsize && canload > 0))return;
    }
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

QString HttpSection::eTag() const
{
    return _etag;
}

void HttpSection::setLastModified(const QDateTime &_dtime)
{
    if(!_dtime.isValid())return;
    lastmodified = _dtime;
}

void HttpSection::setETag(const QString &etag)
{
    _etag = etag;
}

qint64 HttpSection::downSpeed() const
{
    return down_speed;
}

qint64 HttpSection::realSpeed() const
{
    if(!real_speed && !watcher->isNull())
        return (qint64)((double)last_buf_size/(double)watcher->elapsed()*1000.0);

    return real_speed;
}

void HttpSection::setCookie(const QString &cookie)
{
    cookie_string = cookie;
}

QString HttpSection::getCookie() const
{
    return cookie_string;
}

bool HttpSection::freedMemory() const
{
    return (!fl && !soc);
}

QString HttpSection::fileName() const
{
    return flname;
}

QStringList HttpSection::attachedFileName(const QString &cont_dispos) const
{
    if(cont_dispos.indexOf("filename") < 0) return QStringList();
    QStringList words = cont_dispos.split(";");
    for(int i = 0; i<words.size(); ++i)
    {
        if(words.value(i).indexOf("filename")<0)continue;

        QString split_word = "filename=";
        QString codepage;
        if(words.value(i).indexOf("filename*") != -1)
        {
            split_word = "filename\\*=\\w+[-]{0,1}\\w+''";
            codepage = words.value(i).split("filename*=").value(1).split("''").value(0);
        }

        QString str = words.value(i).split(QRegExp(split_word),QString::KeepEmptyParts).value(1);
        if(str.toAscii()[0] == '"' && str.toAscii()[str.toAscii().size()-1] == '"')
            str.replace(QRegExp("(^\")|(\"$)"),"");
        str.replace(QRegExp("[\r\n;]$"),"");

        QStringList out;
        out << str;
        out << codepage;

        return out;
    }
    return QStringList();
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

QHash<QString, QString> HttpSection::getHeader() const
{
    return header;
}

QByteArray HttpSection::ungzipData(QByteArray &data)
{
    if (data.size() <= 4) return QByteArray();

        QByteArray outdata;

        int ret;
        z_stream strm;
        static const int CHUNK_SIZE = 4096;
        char out[CHUNK_SIZE];

        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = data.size();
        strm.next_in = (Bytef*)(data.data());

        ret = inflateInit2(&strm, 47);
        if (ret != Z_OK)
            return QByteArray();
        do {
            strm.avail_out = CHUNK_SIZE;
            strm.next_out = (Bytef*)(out);

            ret = inflate(&strm, Z_NO_FLUSH);

            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return QByteArray();
            }

            outdata.append(out, CHUNK_SIZE - strm.avail_out);
        } while (strm.avail_out == 0);

        inflateEnd(&strm);
        return outdata;
}
