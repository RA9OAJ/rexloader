#include "rexwindow.h"
#include "ui_rexwindow.h"

REXWindow::REXWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::REXWindow)
{
    ui->setupUi(this);

    sched_flag = true;
    QList<int> sz;
    QList<int> sz1;
    QDir libdir(QApplication::applicationDirPath());
    libdir.cdUp();
    pluginDirs << libdir.absolutePath()+"/lib/rexloader/plugins" << QDir::homePath()+"/.rexloader/plugins";

    sz << 800 << 150;
    sz1 << 150 << 800;
    ui->splitter->setSizes(sz);
    ui->splitter_2->setSizes(sz1);

    trayicon = new QSystemTrayIcon(this);
    trayicon->setIcon(QIcon(":/appimages/trayicon.png"));
    trayicon->show();

    apphomedir = QDir::homePath()+"/.rexloader";

    lockProcess();
    openDataBase();
    qDebug()<<loadPlugins();
    qDebug()<<plugfiles;

    scheuler();
    updateTaskSheet();
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
        setEnabled(false);
        int quit_ok = QMessageBox::critical(this,tr("Critical Error!"),tr("Can not open a database file. This is a critical error and the application will close. Check your access privileges on read and write in the home directory, or if exists directory '.rexloader' delete him self."));
        if(quit_ok == QMessageBox::Ok)QTimer::singleShot(0,this,SLOT(close()));
    }

    /*Для отладки*/
    QSqlQuery qr;
    qDebug()<<qr.exec("INSERT INTO tasks (url,datecreate,filename,currentsize,totalsize,downtime,lasterror,mime,tstatus,categoryid,note) VALUES ('url_','2011-06-23T13:00:00','noname.html','100','1000','120','error','mime','0','1','note');");
    /*----------*/

    dbconnect = db.connectionName();
}

int REXWindow::loadPlugins()
{
    for(int i=0; i<pluginDirs.size(); i++)
    {
        QDir dir(pluginDirs.value(i));
        QStringList plg = dir.entryList(QDir::Files);

        for(int y=0; y<plg.size(); y++)
        {
            QPluginLoader plug(pluginDirs.value(i)+"/"+plg.value(y));
            if(!plug.load())continue;
            LoaderInterface *ldr = qobject_cast<LoaderInterface*>(plug.instance());
            pluglist.insert(pluglist.size()+1,ldr);
            plugfiles.insert(pluglist.size(),pluginDirs.value(i)+"/"+plg.value(y));

            QStringList protocols = ldr->protocols();
            for(int x=0; x<protocols.size(); x++)
            {
                if(plugproto.value(protocols.value(x)))continue;
                plugproto.insert(protocols.value(x),pluglist.size());
            }
        }
    }
    return pluglist.size();
}

void REXWindow::lockProcess(bool flag)
{
    QFile fl(apphomedir+"/proc.lock");
    if(flag)
    {
        fl.open(QFile::WriteOnly);
        QString dtime = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");
        fl.write(dtime.toAscii());
        fl.close();
    }
    else fl.remove();
}

void REXWindow::scheuler()
{
    if(!sched_flag)return;
    lockProcess(true);

    QTimer::singleShot(1000,this,SLOT(scheuler()));
}

void REXWindow::updateTaskSheet()
{
    QSqlQuery qr("SELECT * FROM tasks");
   /* while(qr.next())
    {
        int cur_row = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
        QTableWidgetItem *item = new QTableWidgetItem;
        item->setData(Qt::EditRole,qr.value(0).toInt());
        ui->tableWidget->setItem(cur_row,0,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,QDateTime::fromString(qr.value(2).toString(),"yyyy-MM-ddThh:mm:ss"));
        ui->tableWidget->setItem(cur_row,1,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(1).toString());
        ui->tableWidget->setItem(cur_row,3,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(3).toString());
        ui->tableWidget->setItem(cur_row,4,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(9).toInt());
        ui->tableWidget->setItem(cur_row,5,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,100*qr.value(4).toInt()/qr.value(5).toInt());
        item->setText(QString::number(100*qr.value(4).toInt()/qr.value(5).toInt())+"%");
        ui->tableWidget->setItem(cur_row,6,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(5).toInt());
        ui->tableWidget->setItem(cur_row,7,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(4).toString());
        ui->tableWidget->setItem(cur_row,8,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(0).toString());
        ui->tableWidget->setItem(cur_row,0,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(0).toString());
        ui->tableWidget->setItem(cur_row,0,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(0).toString());
        ui->tableWidget->setItem(cur_row,0,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(0).toString());
        ui->tableWidget->setItem(cur_row,0,item);

        item = new QTableWidgetItem;
        item->setData(Qt::DisplayRole,qr.value(0).toString());
        ui->tableWidget->setItem(cur_row,0,item);
    }*/
}

REXWindow::~REXWindow()
{
    delete ui;
    sched_flag = false;


    lockProcess(false);
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
