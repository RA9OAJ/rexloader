#ifndef HASHCALCULATORTHREAD_H
#define HASHCALCULATORTHREAD_H

#include <QThread>
#include <QtCore>

class HashCalculatorThread : public QThread
{
    Q_OBJECT
public:
    HashCalculatorThread(QObject *parent = 0);
    void setFileNames(const QStringList &file_name);
    void stop();

signals:
    void progress(QString file_name, int percent);
    void calcFinished(QString file_name, QString md5, QString sha1);

protected:
    virtual void run();

private:
    QStringList m_file_list;
    const qint64 buf_len;
    bool terminate;
};

#endif // HASHCALCULATORTHREAD_H
