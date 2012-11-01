/*
Project: REXLoader (Downloader), Source file: importmaster.cpp
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

#include "importmaster.h"

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
    foreach(file,filelist)
    {
        emit importStarted();

        QFile fl(file);
        if(!fl.open(QFile::ReadOnly | QFile::Text))
            continue;

        QByteArray buf;
        while(!fl.atEnd())
        {
            if(stopflag)
                break;

            buf.append(fl.readLine(4096));
            ++totalstr;
            emit totalStringLoaded(totalstr);
            QString str(buf);
            QRegExp pattern(QRegExp("(?:)(http|https|ftp):(?://)?"));
            pattern.setPatternSyntax(QRegExp::RegExp2);
            //pattern.setMinimal(true);
            int startpos = str.indexOf(pattern);
            while(startpos >= 0)
            {
                int endpos = str.indexOf(QRegExp("[ \"\\r\\n\\0]"),startpos);
                if(endpos >= 0)
                {
                    emit foundString(str.mid(startpos,endpos-startpos));
                    str.remove(0,endpos+1);
                    buf.clear();
                    buf = str.toAscii();
                }
                else break;

                startpos = str.indexOf(pattern);
            }
            if(buf.size() >= 8192)
                buf.remove(0,4096);
        }
    }
    emit importFinished();
}
