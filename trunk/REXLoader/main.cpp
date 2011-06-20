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



int main(int argc, char *argv[])
{
    checkDatabase();

    QApplication a(argc, argv);

    REXWindow w;
    w.show();

    return a.exec();
}
