#ifndef HASHCALCULATORTHREAD_H
#define HASHCALCULATORTHREAD_H

#include <QThread>
#include <QtCore>

class HashCalculatorThread : public QThread
{
public:
    HashCalculatorThread(const QString &file_name, QObject *parent = 0);
    void setFileName(const QString &file_name);
    QByteArray getMd5() const;
    QByteArray getSha1() const;

protected:
    virtual void run();

private:
    QFile m_file;
    QByteArray m_md5_result;
    QByteArray m_sha1_result;
    const qint64 buf_len;
};

#endif // HASHCALCULATORTHREAD_H
