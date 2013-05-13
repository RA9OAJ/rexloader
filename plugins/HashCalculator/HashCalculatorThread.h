/*
Copyright (C) 2012-2013  Alexey Schukin

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
