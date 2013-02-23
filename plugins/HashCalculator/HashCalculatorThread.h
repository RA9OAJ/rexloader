#ifndef HASHCALCULATORTHREAD_H
#define HASHCALCULATORTHREAD_H

#include <QThread>
#include <QtCore>

class HashCalculatorThread : public QThread
{
    Q_OBJECT
public:
    HashCalculatorThread(QObject *parent = 0);
    void setFileName(const QString &file_name);
    QByteArray getMd5() const;
    QByteArray getSha1() const;
    void stop();

signals:
    void progress(int percent); //состояние расчёта в процентах
    void s_md5(QByteArray hash);
    void s_sha1(QByteArray hash);

protected:
    virtual void run();

private:
    QFile m_file;
    QByteArray m_md5_result;
    QByteArray m_sha1_result;
    const qint64 buf_len;
    bool terminate;
};

#endif // HASHCALCULATORTHREAD_H
