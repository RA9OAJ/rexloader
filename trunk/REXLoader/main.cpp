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
            homeapp.mkpath(homedir+"/.rexloader");
        homeapp.cd(homedir+"/.rexloader");

        bool dbfile = true;
        if(!QFile::exists(homeapp.path()+"/tasks.db"))
        {
            dbfile = false;
            QFile fl(homeapp.path()+"/tasks.db");
            fl.open(QFile::WriteOnly);
            fl.close();
        }
        db.setDatabaseName(homeapp.path()+"/tasks.db");

        if(!db.open())
        {
            qDebug()<<"FUCK!";
            db.setDatabaseName(":memory:");
            if(!db.open())
            {
                //записываем ошибку в error.log
                return;
            }
        }

        if(!dbfile || db.databaseName() == ":memory:")
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
                "categoryid TEXT);");

            if(!flag)
            {
                qDebug()<<qr->lastError();
            }

            qr->clear();
            flag = qr->exec("CREATE TABLE newtasks ("
                            "id INTEGER PRIMARY KEY,"
                            "url TEXT,"
                            "filename TEXT);");

            if(!flag)
            {
                qDebug()<<qr->lastError();
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

    QString homedir = QDir::homePath();
    QDir homeapp;
    QString dbname;
    homeapp.cd(homedir+"./rexloader");

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

        if(!QFile::exists(homeapp.path()+"/tasks.db")) db.setDatabaseName(":memory:");
        else db.setDatabaseName(homeapp.path()+"/task.db");

        if(!db.open())
        {
            //Здесь добавляем сообщение в error.log
        }

        dbname = db.connectionName();

        QUrl url;
        QSqlQuery qr;
        for(int i=1; i<_argv.size(); i++)
        {
            url.clear();
            url.setUrl(_argv.value(i));
            if(url.isValid() && !url.scheme().isEmpty())
            {
                qr.prepare("INSERT INTO newtasks SET url=%1;");
                qr.bindValue(1,url.toString());
                if(!qr.exec())
                {
                    //тут записываем сообщение об ошибке в error.log
                }
            }
            else if(QFile::exists(_argv.value(i)))
            {
                qr.prepare("INSERT INTO newtasks SET filename=%1;");
                qr.bindValue(1,url.toString());
                if(!qr.exec())
                {
                    //тут записываем сообщение об ошибке в error.log
                }
            }
        }
    }
    QSqlDatabase::removeDatabase(dbname);
}

int main(int argc, char *argv[])
{
    checkDatabase();

    QApplication a(argc, argv);
    //addURL(a.arguments());


    REXWindow w;
    w.show();

    return a.exec();
}
