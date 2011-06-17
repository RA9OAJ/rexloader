#ifndef GTCPSOCKET_H
#define GTCPSOCKET_H

#include <QSslSocket>
#include <QTime>
#include <QTimer>
#include <QByteArray>

#include <QDebug>

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

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);
    virtual qint64 readLineData(char *data, qint64 maxlen);

private:
QByteArray *outbuf;
QByteArray *inbuf;
QTime *watcher;

qint64 inspeed;
qint64 outspeed;
qint64 lastSize;
int last_interval;

bool flag;
bool mode;
bool shedule_now;
bool t_flag;

};

#endif // GTCPSOCKET_H
