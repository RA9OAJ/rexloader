#include "HashCalculatorThread.h"


HashCalculatorThread::HashCalculatorThread(QObject *parent):
    QThread(parent), buf_len(4096)
{
    //m_file.setFileName(file_name);
}


void HashCalculatorThread::setFileName(const QString &file_name)
{
    m_file.setFileName(file_name);
    m_md5_result.clear();
    m_sha1_result.clear();
    terminate = false;
}


QByteArray HashCalculatorThread::getMd5() const
{
    return m_md5_result;
}


QByteArray HashCalculatorThread::getSha1() const
{
    return m_sha1_result;
}

void HashCalculatorThread::stop()
{
    terminate = true;
}


void HashCalculatorThread::run()
{
    QCryptographicHash md5_hash(QCryptographicHash::Md5);
    QCryptographicHash sha1_hash(QCryptographicHash::Sha1);

    QByteArray chunk;
    double percent=0.0;
    qint64 file_size=0;
    double delta = 0.0;

    if( !m_file.open( QIODevice::ReadOnly ) )
    {
        qDebug() << "file not open";
        return;
    }
    file_size = m_file.size();
    delta = static_cast<double>(100 / (static_cast<double>(file_size)/static_cast<double>(buf_len)));

    while (!m_file.atEnd())
    {
        chunk = m_file.read(buf_len);
        md5_hash.addData(chunk);
        sha1_hash.addData(chunk);
        emit progress((int)percent);
        percent += delta;
        if (terminate)
        {
            m_file.close();
            return;
        }
    }
    emit progress(100);

    m_md5_result = md5_hash.result();
    m_sha1_result = sha1_hash.result();
    m_file.close();
    emit s_md5(m_md5_result.toHex());
    emit s_sha1(m_sha1_result.toHex());
}
