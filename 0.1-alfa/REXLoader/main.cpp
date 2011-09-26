/*
REXLoader - this cross-platform file downloader from the network.
Copyright (C) 2011  Sarvaritdinov R.

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

#include <QtGui/QApplication>
#include <QTextCodec>
#include "rexwindow.h"
#include <QDebug>

void checkDatabase()
{
    QString homedir = QDir::homePath();
    QDir homeapp;
    QString dbname;
    homeapp.cd(homedir);

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        dbname = db.connectionName();
        if(!homeapp.exists(homedir+"/.rexloader"))
        {
            homeapp.mkpath(homedir+"/.rexloader");
            homeapp.mkpath(homedir+"/.rexloader/plugins");
        }
        homeapp.cd(homedir+"/.rexloader");

        bool dbfile = true;
        if(!QFile::exists(homeapp.path()+"/tasks.db"))dbfile = false;

        db.setDatabaseName(homeapp.path()+"/tasks.db");

        if(!db.open())
        {
             qDebug()<<"0. ";
                //записываем ошибку в error.log
                return;
        }

        if(!dbfile)
        {
            QSqlQuery *qr = new QSqlQuery;
            bool flag = qr->exec("CREATE TABLE tasks ("
                "id INTEGER PRIMARY KEY,"
                "url TEXT,"
                "datecreate TEXT,"
                "filename TEXT,"
                "currentsize TEXT,"
                "totalsize TEXT,"
                "downtime TEXT,"
                "lasterror TEXT,"
                "mime TEXT,"
                "tstatus INTEGER DEFAULT 0,"
                "categoryid TEXT,"
                "speed_avg TEXT,"
                "note TEXT,"
                "priority INTEGER);");

            if(!flag)
            {
                qDebug()<<"1. "<<qr->lastError().text()<<qr->lastQuery();
                //записываем ошибку в error.log
            }

            qr->clear();
            flag = qr->exec("CREATE TABLE newtasks ("
                            "id INTEGER PRIMARY KEY,"
                            "url TEXT,"
                            "filename TEXT);");

            if(!flag)
            {
                qDebug()<<"2. "<<qr->lastError().text()<<qr->lastQuery();
                //записываем ошибку в error.log
            }

            qr->clear();
            flag = qr->exec("CREATE TABLE categories ("
                            "id INTEGER PRIMARY KEY,"
                            "title TEXT,"
                            "dir TEXT,"
                            "extlist TEXT,"
                            "parent_id INTEGER);");

            if(!flag)
            {
                qDebug()<<"3. "<<qr->lastError().text()<<qr->lastQuery();
                //записываем ошибку в error.log
            }

            qr->clear();
            QString queries("INSERT INTO categories (title,dir,parent_id) VALUES ('#downloads','',0);\r\n"
                            "INSERT INTO categories (title,dir,extlist,parent_id) VALUES ('#archives','','7z ace arj bz2 cab cpio deb f gz ha img iso jar lzh lzo lzx rar rpm smc tar xz zip zoo',(SELECT id FROM categories WHERE title='#downloads'));\r\n"
                            "INSERT INTO categories (title,dir,extlist,parent_id) VALUES ('#apps','','bin sh bash exe',(SELECT id FROM categories WHERE title='#downloads'));\r\n"
                            "INSERT INTO categories (title,dir,extlist,parent_id) VALUES ('#audio','','aa aac amr ape asf cda fla flac mp3 mt9 ogg voc wav wma midi mod stm s3m mmf m3u gtp gp3 gp4 gp5',(SELECT id FROM categories WHERE title='#downloads'))\r\n;"
                            "INSERT INTO categories (title,dir,extlist,parent_id) VALUES ('#video','','3gp aaf asf avi bik cpk flv mkv mov mpeg mxf nut nsv ogm mov qt pva smk vivo wmv mp4 vob',(SELECT id FROM categories WHERE title='#downloads'));\r\n"
                            "INSERT INTO categories (title,dir,parent_id) VALUES ('#other','',(SELECT id FROM categories WHERE title='#downloads'));");
            QStringList querstr = queries.split("\r\n");
            for(int i=0; i<querstr.size(); i++)
            {
                flag = qr->exec(querstr.value(i));
                if(!flag)break;
            }

            if(!flag)
            {
                qDebug()<<"4. "<<qr->lastError().text()<<qr->lastQuery()<<querstr;
                //записываем ошибку в error.log
            }

            delete(qr);
        }
        db.close();
    }
    QSqlDatabase::removeDatabase(dbname);
}

void addURL(const QStringList &_argv)
{
    if(_argv.size() <= 1)return;

    QString homedir = QDir::homePath()+"/.rexloader";
    QDir homeapp;
    QString dbname;
    if(!homeapp.cd(homedir))
    {
        //Здесь добавляем сообщение в error.log
        return;
    }

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(homeapp.path()+"/tasks.db");

        if(!db.open())
        {
            //Здесь добавляем сообщение в error.log
            return;
        }

        dbname = db.connectionName();

        QUrl url;
        QSqlQuery qr;
        for(int i=1; i<_argv.size(); i++)
        {
            url.clear();
            url = QUrl::fromEncoded(_argv.value(i).toAscii());

            if((url.isValid() && !url.scheme().isEmpty()) || QFile::exists(url.toString()))
            {
                qr.prepare("INSERT INTO newtasks (url) VALUES (:url)");
                if(url.scheme() == "file") qr.bindValue(":url",_argv.value(i).right(_argv.value(i).size()-7));
                else qr.bindValue(":url",_argv.value(i));
                if(!qr.exec())
                {
                    //тут записываем сообщение об ошибке в error.log
                    return;
                }
            }
            else if(QFile::exists(_argv.value(i)))
            {
                qr.prepare("INSERT INTO newtasks SET filename=%1;");
                qr.bindValue(1,url.toString());
                if(!qr.exec())
                {
                    //тут записываем сообщение об ошибке в error.log
                    return;
                }
            }
        }
    }
    QSqlDatabase::removeDatabase(dbname);
}

bool firstProcess()
{
    QString homeappdir = QDir::homePath()+"/.rexloader";

    if(QFile::exists(homeappdir+"/proc.lock"))
    {
        QFile fl(homeappdir+"/proc.lock");
        fl.open(QFile::ReadOnly);
        QString string = fl.readLine(1024);
        QDateTime proc_time;
        QDateTime cur_time = QDateTime::currentDateTime();
        proc_time = QDateTime::fromString(string,"yyyy-MM-ddThh:mm:ss");

        if(proc_time.secsTo(cur_time) > 5)
        {
            fl.remove();
            return true;
        }
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Sarvaritdinov Ravil");
    QCoreApplication::setApplicationName("REXLoader");

    checkDatabase();

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);
    addURL(a.arguments());
    if(firstProcess())
    {
        QTranslator qt_translator;
        QString trans_file = "qt_"+QLocale::system().name().split("_").value(0)+".qm";
        if(qt_translator.load(trans_file,QLibraryInfo::location(QLibraryInfo::TranslationsPath))) a.installTranslator(&qt_translator);

        REXWindow w;

        w.show();

        return a.exec();
    }
    return 1;
}
