#include <QtGui/QApplication>
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
                            "dir TEXT);");

            if(!flag)
            {
                qDebug()<<"3. "<<qr->lastError().text()<<qr->lastQuery();
                //записываем ошибку в error.log
            }

            qr->clear();
            QString queries("INSERT INTO categories (title,dir) VALUES ('#downloads','');\r\n"
                            "INSERT INTO categories (title,dir) VALUES ('#archives','');\r\n"
                            "INSERT INTO categories (title,dir) VALUES ('#apps','');\r\n"
                            "INSERT INTO categories (title,dir) VALUES ('#audio','')\r\n;"
                            "INSERT INTO categories (title,dir) VALUES ('#video','');\r\n"
                            "INSERT INTO categories (title,dir) VALUES ('#other','');");
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
        qDebug()<<db.databaseName();

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
            url.setUrl(_argv.value(i));
            if(url.isValid() && !url.scheme().isEmpty())
            {qDebug()<<"YES";
                qr.prepare("INSERT INTO newtasks (url) VALUES (:url)");
                qr.bindValue(":url",url.toString());
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
        qDebug()<<proc_time;
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
    checkDatabase();

    QApplication a(argc, argv);
    QApplication::setQuitOnLastWindowClosed(false);
    addURL(a.arguments());
    if(firstProcess())
    {
        REXWindow w;

        w.show();

        return a.exec();
    }
    return 1;
}
