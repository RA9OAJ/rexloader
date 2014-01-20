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

#ifndef GTCPSOCKET_H
#define GTCPSOCKET_H

#include <QSslSocket>
#include <QTime>
#include <QTimer>
#include <QByteArray>

class GTcpSocket : public QSslSocket
{
Q_OBJECT
public:
    explicit GTcpSocket(QObject *parent = 0);
    virtual ~GTcpSocket();

    virtual qint64 bytesAvailable() const;
    qint64 bytesAvailableOnNetwork() const;
    void setSpeed(qint64 in, qint64 out);

    virtual bool canReadLine() const;
    virtual qint64 bytesToWrite() const;

    void setMode(bool mod);
    bool getMode() const;
    bool getShedulerStatus() const;
    bool getTransferStatus() const;
    void connectToHost(const QHostAddress &address, quint16 port, OpenMode mode);
    void connectToHost(const QString &hostName, quint16 port, OpenMode mode);
    void connectToHostEncrypted(const QString &hostName, quint16 port, const QString &sslPeerName, OpenMode mode);
    void connectToHostEncrypted(const QString &hostName, quint16 port, OpenMode mode);

signals:
    void readyToRead();

public slots:
    void transferAct();
    void setDownSpeed(qint64 spd);
    void setUpSpeed(qint64 spd);
    void stopTransfer();
    void startTransfer();

protected slots:
    void connectedAct();
    void sheduler();
    void connectTimeOut();

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);
    virtual qint64 readLineData(char *data, qint64 maxlen);

private:
QByteArray *outbuf;
QByteArray *inbuf;
QTime *watcher;
QTime *timeout;

qint64 inspeed;
qint64 outspeed;
qint64 lastSize;
int last_interval;
int timeout_interval; //таймаут по неактивности сокета в секундах

bool flag;
bool mode;
bool shedule_now;
bool t_flag;

};

#endif // GTCPSOCKET_H
