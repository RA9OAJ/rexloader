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
        if(!QFile::exists(homeapp.path()+"/tasks.db"))dbfile = false;
        db.setDatabaseName(homeapp.path()+"/tasks.db");
        db.open();
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
                "categoryid TEXT);");

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

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    if(!QFile::exists(homeapp.path()+"/tasks.db")) db.setDatabaseName(":tasks.db");
    else db.setDatabaseName(homeapp.path()+"/task.db");

    if(!db.open())
    {
        //Здесь добавляем сообщение в error.log
    }

    QUrl url;
    QSqlQuery qr;
    for(int i=1; i<_argv.size(); i++)
    {
        url.clear();
        url.setEncodedUrl(_argv.value(i));
        if(url.isValid() && !url.scheme().isEmpty())
        {
            //запрос на вставку новой строки в таблицу заданий
        }
    }
}

int main(int argc, char *argv[])
{
    checkDatabase();

    QApplication a(argc, argv);



    REXWindow w;
    w.show();

    return a.exec();
}
