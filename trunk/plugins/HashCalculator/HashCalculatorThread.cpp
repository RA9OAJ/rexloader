#include "HashCalculatorThread.h"

HashCalculatorThread::HashCalculatorThread(QObject *parent):
    QThread(parent), buf_len(4096)
{
}

void HashCalculatorThread::setFileNames(const QStringList &file_name)
{
    m_file_list = file_name;
    terminate = false;
}

void HashCalculatorThread::stop()
{
    terminate = true;
}

void HashCalculatorThread::run()
{
    QCryptographicHash md5_hash(QCryptographicHash::Md5);
    QCryptographicHash sha1_hash(QCryptographicHash::Sha1);
    QByteArray md5_result;
    QByteArray sha1_result;

    QByteArray chunk;
    double percent=0.0;
    qint64 file_size=0;
    double delta = 0.0;
    QString file_name, short_file_name;
    QFile file_object;
    QFileInfo file_info;

    for (int i; i<m_file_list.size(); i++)
    {
        file_name = m_file_list.at(i);
        file_object.setFileName(file_name);
        file_info.setFile(file_object);
        short_file_name = file_info.fileName();
        if(!file_object.open( QIODevice::ReadOnly))
        {
            qDebug() << "file not open";
            return;
        }
        file_size = file_object.size();
        delta = static_cast<double>(100 / (static_cast<double>(file_size)/static_cast<double>(buf_len)));
        while (!file_object.atEnd())
        {
            chunk = file_object.read(buf_len);
            md5_hash.addData(chunk);
            sha1_hash.addData(chunk);
            emit progress(short_file_name, (int)percent);
            percent += delta;
            if (terminate)
            {
                file_object.close();
                return;
            }
        }
        emit progress(short_file_name, 100);
        md5_result = md5_hash.result();
        md5_result = md5_result.toHex();
        sha1_result = sha1_hash.result();
        sha1_result = sha1_result.toHex();
        emit calcFinished(short_file_name, QString(md5_result), QString(sha1_result));
        file_object.close();
        md5_hash.reset();
        sha1_hash.reset();
        percent = 0;
    }
}
