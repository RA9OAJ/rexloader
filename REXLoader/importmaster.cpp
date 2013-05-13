/*
Copyright (C) 2012-2013  Sarvaritdinov R.

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

#include "importmaster.h"
#include "linkextractor.h"

ImportMaster::ImportMaster(QObject *parent) :
    QThread(parent)
{
    stopflag = false;
    totalstr = 0;
}

ImportMaster::~ImportMaster()
{
    if(isRunning())
    {
        stopImport();
        exit(0);
        wait(500);
    }
}

void ImportMaster::addProtocol(const QString &proto)
{
//    if(protocols.isEmpty())
//        protocols = proto;
//    else protocols += QString("|%1").arg(proto);
    protocols << proto;
}

void ImportMaster::stopImport()
{
    stopflag = true;
}

void ImportMaster::import(const QStringList &files)
{
    stopflag = false;
    filelist = files;
    totalstr = 0;
    start();
}

void ImportMaster::run()
{
    QString file;
    LinkExtractor le;
    foreach(file,filelist)
    {
        emit importStarted();

        QFile fl(file);
        if(!fl.open(QFile::ReadOnly | QFile::Text))
            continue;

        // если памяти нехватило, или файл пустой то возвращается
        // пустой QByteArray
        QByteArray buf = fl.readAll();
        if (buf.isEmpty())
            continue;

        QList<ResourceLink> link_list;
        le.setText(QString(buf));
        QStringList name_ext = file.split(".");
        qDebug() << name_ext;
        // если файл без расширения то считаем его текстовым
        if (name_ext.length() == 1)
            link_list = le.extract_txt();
        // иначе смотрим на расширение
        else
        {
            if ((name_ext[1] == QString::fromUtf8("html")) || (name_ext[1] == QString::fromUtf8("htm")))
                link_list = le.extract_html();
            else
                link_list = le.extract_txt();
        }
        ResourceLink link;
        foreach(link, link_list)
        {
            // проверяем наличие протокола из ссылки
            // среди доступных для скачивания
            QStringList list = link.url.split(":");
            if (protocols.indexOf(list[0]) != -1)
                emit foundString(link.url);
        }


//        while(!fl.atEnd())
//        {
//            if(stopflag)
//                break;

//            buf.append(fl.readLine(4096));
//            ++totalstr;
//            emit totalStringLoaded(totalstr);
//            QString str(buf);
//            QRegExp pattern(QRegExp(QString("(?:)(%1):(?://)?").arg(protocols)));
//            pattern.setPatternSyntax(QRegExp::RegExp2);
//            //pattern.setMinimal(true);
//            int startpos = str.indexOf(pattern);
//            while(startpos >= 0)
//            {
//                int endpos = -1;
//                if(fl.atEnd()) endpos = str.size() - 1;
//                else endpos = str.indexOf(QRegExp("[ \"\\r\\n]"),startpos);
//                if(endpos >= 0)
//                {
//                    emit foundString(str.mid(startpos,endpos-startpos));
//                    str.remove(0,endpos+1);
//                    buf.clear();
//                    buf = str.toAscii();
//                }
//                else break;

//                startpos = str.indexOf(pattern);
//            }
//            if(buf.size() >= 8192)
//                buf.remove(0,4096);
//        }
    }
    emit importFinished();
}
