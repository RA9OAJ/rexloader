#include "rexwindow.h"
#include "ui_rexwindow.h"

REXWindow::REXWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::REXWindow)
{
    ui->setupUi(this);

    QList<int> sz;
    QList<int> sz1;
    sz << 800 << 150;
    sz1 << 150 << 800;
    ui->splitter->setSizes(sz);
    ui->splitter_2->setSizes(sz1);

    trayicon = new QSystemTrayIcon(this);
    trayicon->setIcon(QIcon(":/appimages/trayicon.png"));
    trayicon->show();

    //openDataBase();
}

void REXWindow::showNotice(const QString &title, const QString &text, int type)
{

}

void REXWindow::saveSettings()
{

}

void REXWindow::loadSettings()
{

}

void REXWindow::openDataBase()
{
    QString homedir = QDir::homePath()+"/.rexloader";

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(homedir+"/tasks.db");
    if(!db.open())
    {
        trayicon->showMessage(tr("Warning!"),tr("Can not create a database file. This is not a critical error and the application will run, but all data on the tasks will not be saved. Check your access privileges on read and write in the home directory, or if exists directory '.rexloader' delete him self."),QSystemTrayIcon::Warning);
        db.setDatabaseName(":tasks.db");
        db.open();

        QSqlQuery qr;
        qr.prepare("CREATE TABLE tasks ("
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
        if(!qr.exec())
        {
            //!!!!!!!!!!!!!!!
        }
    }
}

void REXWindow::processExists(bool flag)
{
    allright = flag;
}

void REXWindow::showTrayIcon()
{

}

REXWindow::~REXWindow()
{
    delete ui;
}

void REXWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
