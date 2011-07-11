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

    if(!loadPlugins())
    {
        setEnabled(false);
        int quit_ok = QMessageBox::critical(this,windowTitle()+" - "+tr("Critical Error"),tr("Could not find any plug-in application will be closed.\r\n Check the files in the plugins directory '.rexloader' and '/usr/{local/}lib/rexloader/plugins'."));
        if(quit_ok == QMessageBox::Ok)QTimer::singleShot(0,this,SLOT(close()));
    }

    createInterface();

    scheuler();
    updateTaskSheet();
}

void REXWindow::createInterface()
{
    //настраиваем таблицу
    model = new TItemModel(this);
    model->updateModel();
    sfmodel = new QSortFilterProxyModel(this);
    sfmodel->setSortRole(100);
    sfmodel->setSourceModel(model);
    ui->tableView->setModel(sfmodel);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->sortByColumn(0,Qt::AscendingOrder);
    ui->tableView->setAutoScroll(true);
    ui->tableView->horizontalHeader()->setMovable(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    /*ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(4);
    ui->tableView->hideColumn(7);
    ui->tableView->hideColumn(8);
    ui->tableView->hideColumn(10);*/

    //настраиваем панель инструментов
    ui->mainToolBar->addAction(ui->actionAdd_URL);
    ui->mainToolBar->addAction(ui->actionDelURL);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(ui->actionStart);
    ui->mainToolBar->addAction(ui->actionStop);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(ui->actionStartAll);
    ui->mainToolBar->addAction(ui->actionStopAll);
    ui->mainToolBar->addSeparator();

    //кнопка-меню для выбора скорости
    spdbtn = new QToolButton(this);
    QMenu *spdmenu = new QMenu(spdbtn);
    spdmenu->addAction(ui->actionWeryLow);
    spdmenu->addAction(ui->actionLow);
    spdmenu->addAction(ui->actionNormal);
    spdmenu->addAction(ui->actionHigh);
    ui->actionHigh->setChecked(true);
    spdbtn->setMenu(spdmenu);
    spdbtn->setPopupMode(QToolButton::InstantPopup);
    if(ui->actionHigh->isChecked())spdbtn->setIcon(ui->actionHigh->icon());
    else if(ui->actionNormal->isChecked())spdbtn->setIcon(ui->actionNormal->icon());
    else if(ui->actionLow->isChecked())spdbtn->setIcon(ui->actionLow->icon());
    else if(ui->actionWeryLow->isChecked())spdbtn->setIcon(ui->actionWeryLow->icon());
    ui->mainToolBar->addWidget(spdbtn);

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
        int quit_ok = QMessageBox::critical(this,windowTitle()+" - "+tr("Critical Error"),tr("Can not open a database file.\r\n This is a critical error and the application will close.\r\n Check your access privileges on read and write in the home directory, or if exists directory '.rexloader' delete him self."));
        if(quit_ok == QMessageBox::Ok)QTimer::singleShot(0,this,SLOT(close()));
    }

    /*Для отладки*/
    QSqlQuery qr;
    qr.exec("INSERT INTO tasks (url,datecreate,filename,currentsize,totalsize,downtime,lasterror,mime,tstatus,categoryid,note) VALUES ('url_','2011-06-23T13:00:00','noname.html','100','1000','120','error','mime','0','1','note');");
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
    model->updateModel();
    ui->tableView->update();
}

REXWindow::~REXWindow()
{
    delete ui;
    sched_flag = false;

    lockProcess(false);
}

QStringList REXWindow::sizeForHumans(qint64 sz)
{
    QStringList outstrings;
    if(sz >= 1073741824)outstrings << QString::number(sz/1073741824) << tr(" GB");
    else if(sz >= 1048576)outstrings << QString::number(sz/1048576) << tr(" MB");
    else if(sz >= 1024)outstrings << QString::number(sz/1024) << tr(" kB");
    else outstrings << QString::number(sz) << tr(" bytes");

    return outstrings;
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
