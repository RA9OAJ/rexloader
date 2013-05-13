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

#include "gtcpsocket.h"

GTcpSocket::GTcpSocket(QObject *parent) :
    QSslSocket(parent)
{
    flag = true;
    t_flag = true;
    shedule_now = false;
    mode = true;

    outbuf = new QByteArray();
    inbuf = new QByteArray();
    outbuf->clear();
    inbuf->clear();
    watcher = new QTime();
    timeout = new QTime();

    inspeed = 0;
    outspeed = 0;
    last_interval = 0;
    timeout_interval = 30;

    connect(this, SIGNAL(connected()), this, SLOT(connectedAct()));
}

GTcpSocket::~GTcpSocket()
{
    delete(inbuf);
    delete(outbuf);
    delete(watcher);
    delete(timeout);
}

void GTcpSocket::connectedAct()
{
    shedule_now = false;
    t_flag = true;
    flag = false;
    sheduler();
}

qint64 GTcpSocket::bytesAvailable() const
{
    return (qint64)inbuf->size();
}

qint64 GTcpSocket::bytesAvailableOnNetwork() const
{
    return QSslSocket::bytesAvailable();
}

void GTcpSocket::setSpeed(qint64 in, qint64 out)
{
    inspeed = in;
    outspeed = out;

    lastSize = readBufferSize();
    if(inspeed*2 < readBufferSize())return;
    setReadBufferSize(in*2);
}

void GTcpSocket::setDownSpeed(qint64 spd)
{

    inspeed = spd;

    if(inspeed*2 < readBufferSize())return;

    lastSize = readBufferSize();
    setReadBufferSize(inspeed*2);

}

void GTcpSocket::setUpSpeed(qint64 spd)
{
    outspeed = spd;
}

void GTcpSocket::sheduler()
{    
    if(!shedule_now && mode)
        transferAct();

    if(mode) {QTimer::singleShot(10, this, SLOT(sheduler()));}
}

void GTcpSocket::transferAct()
{
    if(shedule_now)return; //если уже идет обмен с внешними буферами, то выходим
    if(!t_flag) return; //если флаг паузы, то выходим - не переносим ничего и никуда

    shedule_now = true; //устанавливаем признак выполнения данного метода
    int interval = 1000; //начальный интервал для подсчета лимита байт на скачивания/передачи

    if(this->state() != QAbstractSocket::ConnectedState && QSslSocket::bytesAvailable() == 0)
    {
        shedule_now = false;
        if(inbuf->size() > 0)emit readyToRead();
        return;
    } //если сокет был отсоединен и нету данных во внешнем буфеое, то выходим

    if(!watcher->isNull()) //если таймер запускается не впервые
        interval = qMin(watcher->elapsed(), interval);//то выбираем наименьшее между начальным значением и интервалом с момента прошлой передачи данных
    watcher->start();

    qint64 inLimit = (inspeed * interval)/1000; //высчитываем лимит приема/передачи
    qint64 outLimit = (outspeed * interval)/1000;
    if(outLimit == 0) outLimit = outbuf->size(); // если не утсновлено ограничение на отдачу, то передается все cодержимое выходного буфера

    qint64 bytesToRead;

    if(state() != QAbstractSocket::ConnectedState)
    {
        bytesToRead = QSslSocket::bytesAvailable();

        if(bytesToRead > 2097152 /*2MB*/)bytesToRead = 2097152;
    }
    else
    {
        bytesToRead = qMin<qint64>(inLimit, QSslSocket::bytesAvailable());
        if(QSslSocket::bytesAvailable() > 0) timeout->start();
        else if(timeout->elapsed() > timeout_interval*1000 && !timeout->isNull())
        {
            emit error(QSslSocket::SocketTimeoutError);
            close();
            shedule_now = false;
            return;
        }
    }
    qint64 bytesToWrite = qMin<qint64>(outLimit, outbuf->size());

    if(inspeed*2 < readBufferSize())
    {
        qint64 dif = QSslSocket::bytesAvailable() - inspeed*2;
        if(dif > 50) setReadBufferSize(readBufferSize()-50);
        else setReadBufferSize(readBufferSize()-dif);
    }

    if(inspeed != 0)
    {
        int old_size = inbuf->size();
        inbuf->resize(old_size + bytesToRead);
        QSslSocket::readData(inbuf->data() + old_size, bytesToRead);
    }
    QSslSocket::writeData(outbuf->data(), bytesToWrite);
    outbuf->remove(0, bytesToWrite);
    flush();

    shedule_now = false;
    if(bytesToRead > 0 && inspeed !=0 )emit readyToRead();
    else if (inspeed == 0 && QSslSocket::bytesAvailable()!= 0) emit readyToRead();
}

qint64 GTcpSocket::writeData(const char *data, qint64 len)
{
    if (outspeed == 0 && outbuf->isEmpty())
        return QSslSocket::writeData(data, len); //если исходящий буфер пуст и ограничения на скорость отдачи нет, то отправляется напрямую

    outbuf->append(data, len);
    return len;
}

qint64 GTcpSocket::readData(char *data, qint64 maxlen)
{
    qint64 toRead = this->bytesAvailable();

    if (inbuf->isEmpty() && inspeed == 0)
        return QSslSocket::readData(data, maxlen); //если ограничений на скорость скачивания нет и вхожящий буфер пуст, то качаем напрямую из внешнего буфера

    toRead = qMin(toRead, maxlen);

    memcpy(data, inbuf->constData(), toRead);
    inbuf->remove(0,toRead);


    return toRead;
}

qint64 GTcpSocket::readLineData(char *data, qint64 maxlen)
{
    if(inspeed == 0)
    {
        if(inbuf->isEmpty()) return QSslSocket::readLineData(data, maxlen);
        if((inbuf->indexOf(0xA)) == -1)
        {
            if(!QSslSocket::canReadLine())return 0;
            QByteArray _buf;
            _buf.resize(maxlen - inbuf->size());
            if(QSslSocket::readLineData(_buf.data(), _buf.size()) < 0) return -1;
            inbuf->append(_buf);
        }

    }

    int strlen = (inbuf->indexOf(0xA));
    if(strlen == -1)return 0;
    strlen += 1;
    qint64 toRead = maxlen;

    toRead = qMin<qint64>(toRead, strlen);

    memcpy(data, inbuf->constData(), toRead);
    inbuf->remove(0,toRead);

    return toRead;
}

qint64 GTcpSocket::bytesToWrite() const
{
    if (outbuf->isEmpty() && outspeed == 0)
        return QSslSocket::bytesToWrite();

    return (qint64)outbuf->size();
}

bool GTcpSocket::canReadLine() const
{
    if((inbuf->indexOf(0xA)) != -1)return true;

    if (inspeed == 0)
        return QSslSocket::canReadLine();

    return false;
}

bool GTcpSocket::getMode() const
{
    return mode;
}

void GTcpSocket::setMode(bool mod)
{
    mode = mod;
}

bool GTcpSocket::getShedulerStatus() const
{
    return shedule_now;
}

void GTcpSocket::startTransfer()
{
    t_flag = true;
}

void GTcpSocket::stopTransfer()
{
    t_flag = false;
}

bool GTcpSocket::getTransferStatus() const
{
    return t_flag;
}
