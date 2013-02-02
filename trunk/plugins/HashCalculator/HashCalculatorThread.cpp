#include "HashCalculatorThread.h"


HashCalculatorThread::HashCalculatorThread(const QString &file_name, QObject *parent):
    QThread(parent), buf_len(4096)
{
    m_file.setFileName(file_name);
}


void HashCalculatorThread::setFileName(const QString &file_name)
{
    m_file.setFileName(file_name);
    m_md5_result.clear();
    m_sha1_result.clear();
}


QByteArray HashCalculatorThread::getMd5() const
{
    return m_md5_result;
}


QByteArray HashCalculatorThread::getSha1() const
{
    return m_sha1_result;
}


void HashCalculatorThread::run()
{
    QCryptographicHash md5_hash(QCryptographicHash::Md5);
    QCryptographicHash sha1_hash(QCryptographicHash::Sha1);

    QByteArray chunk;

    if( !m_file.open( QIODevice::ReadOnly ) )
    {
        qDebug() << "file not open";
        return;
    }


    do {
        chunk = m_file.read(buf_len);
        md5_hash.addData(chunk);
        sha1_hash.addData(chunk);
    } while ( chunk.isEmpty() );

    m_md5_result = md5_hash.result();
    m_sha1_result = sha1_hash.result();
    m_file.close();
}
