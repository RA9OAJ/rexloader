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
#include <QMessageBox>
#include <QTextCodec>
#include <QSharedMemory>
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
        if(!homeapp.exists(homedir+"/.config"))
            homeapp.mkpath(homedir+"/.config");

        homeapp.cd(homedir+"/.config");

        QDir olddir(QDir::homePath()+"/.rexloader");
        if(olddir.exists())
        {
            olddir.rename(olddir.absolutePath(), homeapp.absolutePath()+"/rexloader");
            QMessageBox::information(0,QObject::tr("Перенос настроек"),QObject::tr("Файлы настроек REXLoader были перенесены в ~/.config/rexloader"));
        }

        if(!homeapp.exists(homeapp.absolutePath()+"/rexloader"))
        {
            homeapp.mkpath(homeapp.absolutePath()+"/rexloader");
            homeapp.mkpath(homeapp.absolutePath()+"/rexloader");
        }
        homeapp.cd(homeapp.absolutePath()+"/rexloader");
        if(!homeapp.exists(homeapp.path()+"/logs"))
            homeapp.mkpath(homeapp.path()+"/logs");
        if(!homeapp.exists(homeapp.path()+"/locales"))
            homeapp.mkpath(homeapp.path()+"/locales");

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
                "priority INTEGER,"
                "params TEXT,"
                "plane_id INTEGER,"
                "arch INTEGER);");

            if(!flag)
            {
                qDebug()<<"1. "<<qr->lastError().text()<<qr->lastQuery();
                //записываем ошибку в error.log
            }

            qr->clear();
            flag = qr->exec("CREATE TABLE newtasks ("
                            "id INTEGER PRIMARY KEY,"
                            "url TEXT,"
                            "filename TEXT,"
                            "params TEXT);");

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

            qr->clear();
            flag = qr->exec("CREATE TABLE planes ("
                            "id INTEGER PRIMARY KEY,"
                            "title TEXT,"
                            "startdatetime TEXT,"
                            "enddatetime TEXT);");

            if(!flag)
            {
                qDebug()<<"5. "<<qr->lastError().text()<<qr->lastQuery()<<querstr;
                //записываем ошибку в error.log
            }

            qr->clear();
            flag = qr->exec("CREATE TABLE mirrors ("
                            "id INTEGER PRIMARY KEY,"
                            "url TEXT,"
                            "tid INTEGER,"
                            "prior INTEGER,"
                            "params TEXT);");

            if(!flag)
            {
                qDebug()<<"6. "<<qr->lastError().text()<<qr->lastQuery()<<querstr;
                //записываем ошибку в error.log
            }

            delete qr;
        }

        { // секция кода для обновления БД с предыдущей версии программы
            QSqlQuery *qr = new QSqlQuery;

            //если нет таблицы с планами, то создаем её
            bool flag = qr->exec("CREATE TABLE IF NOT EXISTS planes ("
                "id INTEGER PRIMARY KEY,"
                "title TEXT,"
                "startdatetime TEXT,"
                "enddatetime TEXT);");

            if(!flag)
            {
                qDebug()<<"7. "<<qr->lastError().text()<<qr->lastQuery();
                //записываем ошибку в error.log
            }

            //если в таблице tasks < 17 полей, то добавляем 17 поле - arch
            qr->clear();
            flag = qr->exec("SELECT * FROM tasks");

            if(!flag)
            {
                qDebug()<<"8. "<<qr->lastError().text()<<qr->lastQuery();
                //записываем ошибку в error.log
            }
            else if(qr->record().count() < 16)
            {
                qr->clear();
                flag = qr->exec("ALTER TABLE tasks ADD COLUMN arch INTEGER");

                if(!flag)
                {
                    qDebug()<<"9. "<<qr->lastError().text()<<qr->lastQuery();
                    //записываем ошибку в error.log
                }
            }

            qr->clear();
            flag = qr->exec("CREATE TABLE IF NOT EXISTS mirrors ("
                            "id INTEGER PRIMARY KEY,"
                            "url TEXT,"
                            "tid INTEGER,"
                            "prior INTEGER,"
                            "params TEXT);");

            if(!flag)
            {
                qDebug()<<"10. "<<qr->lastError().text()<<qr->lastQuery();
                //записываем ошибку в error.log
            }

            delete qr;
        }

        db.close();
    }
    QSqlDatabase::removeDatabase(dbname);
}

void addURL(const QStringList &_argv)
{
    if(_argv.size() <= 1)return;

    QString homedir = QDir::homePath()+"/.config/rexloader";
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
        int par_id = 0;
        QString params,address;
        for(int i=1; i<_argv.size(); i++)
        {
            if(_argv.value(i) == "-o" || _argv.value(i) == "--options")
            {
                par_id = 1;
                continue;
            }
            else if(_argv.value(i) == "-u" || _argv.value(i) == "--url")
            {
                par_id = 0;
                continue;
            }

            switch(par_id)
            {
            case 0:
                address = _argv.value(i);
                break;
            case 1:
                if(params.isEmpty())
                    params += _argv.value(i);
                else
                    params += QString("\n\n%1").arg(_argv.value(i));
                break;
            default:
                continue;
            }
        }

        if(address.isEmpty())
            return;

        url.clear();
        url = QUrl::fromEncoded(address.toUtf8());

        if((url.isValid() && !url.scheme().isEmpty()) || QFile::exists(address))
        {
            qr.prepare("INSERT INTO newtasks (url, params) VALUES (:url, :params)");
            if(url.scheme() == "file") qr.bindValue(":url",address.right(address.size()-7));
            else qr.bindValue(":url",url.toString()/*_argv.value(i)*/);
            qr.bindValue(":params",params);
            if(!qr.exec())
            {
                //тут записываем сообщение об ошибке в error.log
                return;
            }
        }
        else if(QFile::exists(address))
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
    QSqlDatabase::removeDatabase(dbname);
}

bool firstProcess()
{
    QSharedMemory lock_mem("rexloader");
    if(!lock_mem.create(32))
        lock_mem.attach();

    if(lock_mem.isAttached())
    {
        QByteArray data(( const char*)lock_mem.data(),lock_mem.size());
        QString dtime;
        //data.setRawData((char*)lock_mem.data(),lock_mem.size());
        dtime = data;

        QDateTime proc_time;
        QDateTime cur_time = QDateTime::currentDateTime();
        proc_time = QDateTime::fromString(dtime,"yyyy-MM-ddThh:mm:ss");

        if(proc_time.secsTo(cur_time) > 5)
            return true;
    }
    return false;
}

void setAppLanguage(QApplication *app)
{
    QString homedir = QDir::homePath()+"/.config/rexloader/locales";
    QDir libdir(QApplication::applicationDirPath());
    libdir.cdUp();

    QString systransfile = "qt_"+QLocale::system().name().split("_").value(0)+".qm";
    QString apptransfile = QLocale::system().name()+".qm";

    QTranslator *translator = new QTranslator(app);
    if(translator->load(systransfile,QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app->installTranslator(translator);
    else if(translator->load(systransfile,homedir))
        app->installTranslator(translator);
    else delete translator;

    translator = new QTranslator(app);
    if(translator->load(apptransfile,homedir))
        app->installTranslator(translator);
    else if(translator->load(apptransfile,libdir.absolutePath()+"/share/rexloader/locales"))
        app->installTranslator(translator);
    else
    {
        if(apptransfile == "ru_RU.qm")
        {
            delete translator;
            return;
        }

        if(translator->load("en_US.qm",libdir.absolutePath()+"/share/rexloader/locales"))
            app->installTranslator(translator);
        else delete translator;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Sarvaritdinov Ravil");
    QCoreApplication::setApplicationName("REXLoader");

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);
    checkDatabase();

    QApplication::setQuitOnLastWindowClosed(false);
    addURL(a.arguments());
    if(firstProcess())
    {
        /*QTranslator qt_translator;
        QString trans_file = "qt_"+QLocale::system().name().split("_").value(0)+".qm";
        if(qt_translator.load(trans_file,QLibraryInfo::location(QLibraryInfo::TranslationsPath))) a.installTranslator(&qt_translator);
        */
        setAppLanguage(&a);

        REXWindow w;

        w.show();

        return a.exec();
    }
    return 0;
}
