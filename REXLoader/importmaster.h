/*
Project: REXLoader (Downloader), Source file: importmaster.h
Copyright (C) 2012  Sarvaritdinov R.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef IMPORTMASTER_H
#define IMPORTMASTER_H

#include <QThread>
#include <QRegExp>
#include <QStringList>
#include <QDebug>
#include <QFile>

class ImportMaster : public QThread
{
    Q_OBJECT
public:
    explicit ImportMaster(QObject *parent = 0);
    ~ImportMaster();
    void addProtocol(const QString &proto);
    
signals:
    void foundString(const QString &str);
    void importStarted();
    void importFinished();
    void totalStringLoaded(qint64 total);
    
public slots:
    void stopImport();
    void import(const QStringList &files);
    
protected:
    virtual void run();

private:
    bool stopflag;
    QStringList filelist;
    qint64 totalstr;
    //QString protocols;
    QStringList protocols;
};

#endif // IMPORTMASTER_H
