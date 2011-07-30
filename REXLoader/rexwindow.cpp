/*
Project: REXLoader (Downloader), Source file: REXWindow.cpp
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

#include "rexwindow.h"
#include "ui_rexwindow.h"

REXWindow::REXWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::REXWindow)
{
    ui->setupUi(this);

    sched_flag = true;
    clip_autoscan = true;
    max_tasks = 9;
    max_threads = 3;
    down_speed = 2048*8; // 2Mbps

    QList<int> sz;
    QList<int> sz1;
    QDir libdir(QApplication::applicationDirPath());
    libdir.cdUp();
    pluginDirs << libdir.absolutePath()+"/lib/rexloader/plugins" << QDir::homePath()+"/.rexloader/plugins";

    downDir = QDir::homePath()+"/Downloads";
    if(!QDir().exists(downDir))
        QDir().mkpath(downDir);

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

    QTimer::singleShot(250,this,SLOT(scheuler()));
    updateTaskSheet();
}

void REXWindow::createInterface()
{
    //настраиваем таблицу
    model = new TItemModel(this);
    //model->setEditStrategy(QSqlTableModel::OnManualSubmit);
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
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(4);
    ui->tableView->hideColumn(7);
    ui->tableView->hideColumn(8);
    ui->tableView->hideColumn(10);
    ui->tableView->hideColumn(13);

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

    //настраиваем статусбар
    QWidget *widget = new QWidget(ui->statusBar);
    widget->setObjectName("widget");
    QHBoxLayout *lay = new QHBoxLayout(widget);
    QLabel *statusIcon = new QLabel(widget);
    statusIcon->setObjectName("statusIcon");
    QLabel *priorityIcon = new QLabel(widget);
    priorityIcon->setObjectName("priorityIcon");
    QLabel *urllbl = new QLabel(widget);
    urllbl->setObjectName("urllbl");
    urllbl->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    urllbl->setMinimumWidth(75);
    urllbl->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    urllbl->setScaledContents(true);
    QLabel *speed = new QLabel(widget);
    speed->setObjectName("speed");
    speed->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    speed->setMaximumWidth(75);
    QProgressBar *prgBar = new QProgressBar(widget);
    prgBar->setObjectName("prgBar");
    QLabel *timeleft = new QLabel(widget);
    timeleft->setObjectName("timeleft");
    timeleft->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    timeleft->setMaximumWidth(300);
    QLabel *lasterror = new QLabel(widget);
    lasterror->setObjectName("lasterror");
    lasterror->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    lasterror->setMaximumWidth(300);
    lasterror->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    QSpacerItem *spacer = new QSpacerItem(100,10,QSizePolicy::Expanding);
    QLabel *onplayIcon = new QLabel(ui->statusBar);
    onplayIcon->setPixmap(QPixmap(":/appimages/start_16x16.png"));
    QLabel *onplay = new QLabel(ui->statusBar);
    onplay->setObjectName("onplay");
    QLabel *onpauseIcon = new QLabel(ui->statusBar);
    onpauseIcon->setPixmap(QPixmap(":/appimages/pause_16x16.png"));
    QLabel *onpause = new QLabel(ui->statusBar);
    onpause->setObjectName("onpause");
    QLabel *onqueueIcon = new QLabel(ui->statusBar);
    onqueueIcon->setPixmap(QPixmap(":/appimages/queue_16x16.png"));
    QLabel *onqueue = new QLabel(ui->statusBar);
    onqueue->setObjectName("onqueue");
    QLabel *onerrorIcon = new QLabel(ui->statusBar);
    onerrorIcon->setPixmap(QPixmap(":/appimages/error_16x16.png"));
    QLabel *onerror = new QLabel(ui->statusBar);
    onerror->setObjectName("onerror");
    onerror->setMinimumWidth(20);

    lay->addWidget(statusIcon);
    lay->addWidget(priorityIcon);
    lay->addWidget(urllbl);
    lay->addWidget(speed);
    lay->addWidget(prgBar);
    lay->addWidget(timeleft);
    lay->addWidget(lasterror);
    lay->addSpacerItem(spacer);
    ui->statusBar->addWidget(widget);
    ui->statusBar->addPermanentWidget(onplayIcon);
    ui->statusBar->addPermanentWidget(onplay);
    ui->statusBar->addPermanentWidget(onpauseIcon);
    ui->statusBar->addPermanentWidget(onpause);
    ui->statusBar->addPermanentWidget(onqueueIcon);
    ui->statusBar->addPermanentWidget(onqueue);
    ui->statusBar->addPermanentWidget(onerrorIcon);
    ui->statusBar->addPermanentWidget(onerror);

    //соединяем сигналы и слоты
    connect(ui->actionAdd_URL,SIGNAL(triggered()),this,SLOT(showAddTaskDialog()));
    connect(ui->actionDelURL,SIGNAL(triggered()),this,SLOT(deleteTask()));
    connect(ui->actionStart,SIGNAL(triggered()),this,SLOT(startTask()));
    connect(ui->actionStop,SIGNAL(triggered()),this,SLOT(stopTask()));
    connect(ui->actionStartAll,SIGNAL(triggered()),this,SLOT(startAllTasks()));
    connect(ui->actionStopAll,SIGNAL(triggered()),this,SLOT(stopAllTasks()));
    connect(ui->tableView,SIGNAL(clicked(QModelIndex)),this,SLOT(updateStatusBar()));

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

    //настроиваем значок в трее
    QMenu *traymenu = new QMenu(this);
    traymenu->setObjectName("traymenu");
    QAction *trayact = new QAction(this);
    trayact->setObjectName("exitAct");
    trayact->setText(tr("Exit"));
    connect(trayact,SIGNAL(triggered()),this,SLOT(close()));
    traymenu->addAction(trayact);

    trayicon->setContextMenu(traymenu);
    connect(trayicon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(showHideSlot(QSystemTrayIcon::ActivationReason)));


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

void REXWindow::scanClipboard()
{
    if(!clip_autoscan)return;

    const QClipboard *clipbrd = QApplication::clipboard();

    QUrl url(clipbrd->text());

    if(url.isValid() && plugproto.contains(url.scheme().toLower()) && clipbrd->text() != clip_last)
    {
        clip_last = clipbrd->text();
        AddTaskDialog *dlg = new AddTaskDialog(downDir, 0);
        connect(dlg,SIGNAL(addedNewTask()),this,SLOT(updateTaskSheet()));
        dlg->setValidProtocols(plugproto);
        dlg->setNewUrl(clipbrd->text());
        dlg->setWindowFlags(Qt::WindowStaysOnTopHint);
        dlg->show();
        dlg->setFocus();
    }
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

    QSqlQuery qr;
    qr.prepare("UPDATE tasks SET tstatus=:newstatus WHERE tstatus=:tstatus;");
    qr.bindValue("newstatus",LInterface::ON_PAUSE);
    qr.bindValue("tstatus",LInterface::ON_LOAD);

    if(!qr.exec())
    {
        //записываем ошибку в журнал
        qDebug()<<"void REXWindow::openDataBase(1)" + qr.executedQuery() + " Error:" + qr.lastError().text();
    }

    dbconnect = db.connectionName();
}

void REXWindow::scanNewTaskQueue()
{
    QSqlQuery qr;
    if(!qr.exec("SELECT * FROM newtasks;"))
    {
        // запись в журнал ошибок
        qDebug()<<"Error: void REXWindow::scanNewTaskQueue(): "<<qr.lastError().text();
        return;
    }

    AddTaskDialog *dlg;
    while(qr.next())
    {

        if(qr.value(1).toString() != "")
        {
            QUrl url(qr.value(1).toString());
            if((url.scheme().toLower() == "file" || url.scheme().isEmpty()) && QFile::exists(qr.value(1).toString()))
            {
                //тут считываем данные о загрузке из недокачанного файла
            }
            else if(plugproto.contains(url.scheme().toLower()))
            {
                dlg = new AddTaskDialog(downDir, 0);
                connect(dlg,SIGNAL(addedNewTask()),this,SLOT(updateTaskSheet()));
                dlg->setValidProtocols(plugproto);
                dlg->setNewUrl(qr.value(1).toString());
                dlg->setWindowFlags(Qt::WindowStaysOnTopHint);
                dlg->show();
            }
        }

        QSqlQuery qr1;
        qr1.prepare("DELETE FROM newtasks WHERE id=:id");
        qr1.bindValue("id",qr.value(0));

        if(!qr1.exec())
        {
            //запись в журнал ошибок
            qDebug()<<"Error: void REXWindow::scanNewTaskQueue(): "<<qr.lastError().text();
            return;
        }

    }
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
                ldr->setMaxSectionsOnTask(max_threads);
                ldr->setDownSpeed(down_speed*1024/8);
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

    scanNewTaskQueue();
    scanClipboard();
    syncTaskData();
    manageTaskQueue();

    QTimer::singleShot(1000,this,SLOT(scheuler()));
}

void REXWindow::trans_scheduler()
{
    emit transAct();
    QTimer::singleShot(50,this,SLOT(trans_scheduler()));
}

void REXWindow::updateTaskSheet()
{
    model->silentUpdateModel();
    model->clearCache();
}

void REXWindow::startTrayIconAnimaion()
{

}

void REXWindow::stopTrayIconAnimation()
{

}

void REXWindow::updateTrayIcon()
{

}

void REXWindow::showAddTaskDialog()
{
    AddTaskDialog *dlg = new AddTaskDialog(downDir, this);
    connect(dlg,SIGNAL(addedNewTask()),this,SLOT(updateTaskSheet()));
    dlg->setValidProtocols(plugproto);
    dlg->show();
}

void REXWindow::showHideSlot(QSystemTrayIcon::ActivationReason type)
{
    if(type == QSystemTrayIcon::DoubleClick)
    {
        if(isVisible())setHidden(true);
        else setHidden(false);
    }
}

void REXWindow::startTaskNumber(int id_row, const QUrl &url, const QString &filename, qint64 totalload)
{
    if(!plugproto.contains(url.scheme().toLower())) return;

    QFileInfo flinfo(filename);
    int id_proto = plugproto.value(url.scheme().toLower());
    int id_task = 0;

    if(totalload > 0) //если файл на докачке, а не новый
    {

        if(QFile::exists(filename)) //если локальный файл существует
        {
            id_task = pluglist.value(id_proto)->loadTaskFile(url.toString()); // id задачи

            if(id_task) //если плагин удачно прочитал метаданные и добавил задачу
            {
                tasklist.insert(id_row, id_task + id_proto*100);
                pluglist.value(id_proto)->setTaskFilePath(id_task,flinfo.absolutePath());
                pluglist.value(id_proto)->startDownload(id_task);
                updateTaskSheet();
                return;
            }
            else
            {
                //тут запись в журнал ошибок или запрос на счет того, желает ли пользователь снова закачать файл с самогог начала

            }
        }
    }

    //дальше идет код, который добавляет новую закачку на основе URL в очередь плагина
    id_task = pluglist.value(id_proto)->addTask(url.toString());

    if(!id_task)
    {
        //тут запись в журнал ошибок
        return;
    }

    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("UPDATE tasks SET downtime='', lasterror='', speed_avg='' WHERE id=:id");
    qr.bindValue("id",id_row);
    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::startTaskNumber(1): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
    }

    tasklist.insert(id_row, id_task + id_proto*100);
    pluglist.value(id_proto)->setTaskFilePath(id_task,flinfo.absolutePath());
    pluglist.value(id_proto)->startDownload(id_task);
    updateTaskSheet();
}

void REXWindow::deleteTask()
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())return; //если ничего невыделено, то выходим

    if(select->selectedRows().length() > 1) //если выделено более 1 строки
    {
        EMessageBox dlg(this);
        dlg.setIcon(EMessageBox::Warning);
        dlg.setText(tr("Select more than one task."));
        dlg.setInformativeText("To delete an entire group of selected tasks, click <b>\"Ok\"</b> or <b>\"Cancel\"</b> to cancel.");
        dlg.setStandardButtons(EMessageBox::Ok | EMessageBox::Cancel);
        dlg.setDefaultButton(EMessageBox::Cancel);

        int result = dlg.exec();
        if(result == EMessageBox::Cancel)return;
    }

    stopTask(); //останавливаем задания, попавшие в выделение

    QSqlQuery qr(QSqlDatabase::database());

    QString where;
    for(int i=0; i < select->selectedRows().length(); i++)
    {
        if(!i)
            where += QString(" id=%1").arg(select->selectedRows().value(i).data(Qt::DisplayRole).toString());
        else
            where += QString(" OR id=%1").arg(select->selectedRows().value(i).data(Qt::DisplayRole).toString());
    }

    qr.prepare("DELETE FROM tasks WHERE"+where);

    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::deleteTask(): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
    }

    model->updateModel(); //обновляем таблицу задач
    model->clearCache();
    manageTaskQueue();
}

void REXWindow::startTask()
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())return; //если ничего невыделено, то выходим

    for(int i=0; i < select->selectedRows().length(); i++)
    {
        int id_row = select->selectedRows(0).value(i).data(100).toInt(); // id записи в базе данных
        int tstatus = select->selectedRows(9).value(i).data(100).toInt(); //статус в базе данных

        switch(tstatus)
        {
        case LInterface::ERROR_TASK:
        case LInterface::ON_PAUSE: break;
        case -100:
        default: continue;
        }

        QSqlQuery qr(QSqlDatabase::database());
        qr.prepare("UPDATE tasks SET tstatus=-100, lasterror='' WHERE id=:id");
        qr.bindValue("id",id_row);

        if(!qr.exec())
        {
            //запись в журнал ошибок
            qDebug()<<"void REXWindow::startTask(1): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
        }
    }
    updateTaskSheet();
    manageTaskQueue();
    syncTaskData();
}

void REXWindow::startAllTasks()
{
    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("UPDATE tasks SET tstatus=-100, lasterror='' WHERE tstatus=0");
    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::startAllTasks(1): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
        return;
    }
    updateTaskSheet();
    manageTaskQueue();
    syncTaskData();
}

void REXWindow::stopTask()
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())return; //если ничего не выделено, то выходим

    for(int i=0; i < select->selectedRows().length(); i++)
    {
        QString filepath = select->selectedRows(3).value(i).data(100).toString(); //путь к локальному файлу закачки
        QUrl _url(select->selectedRows(1).value(i).data(100).toString()); //URL закачки
        int id_row = select->selectedRows(0).value(i).data(100).toInt(); // id записи в базе данных
        int tstatus = select->selectedRows(9).value(i).data(100).toInt(); //статус в базе данных

        switch(tstatus)
        {
        case LInterface::ON_LOAD:
        case LInterface::SEND_QUERY:
        case LInterface::ACCEPT_QUERY: break;
        default: continue;
        }

        int id_proto = plugproto.value(_url.scheme().toLower()); // id плагина с соответствующим протоколом
        int id_task = tasklist.value(id_row)%100; // id задачи

        pluglist.value(id_proto)->stopDownload(id_task);
    }
    updateTaskSheet();
    manageTaskQueue();
    syncTaskData();
}

void REXWindow::stopAllTasks()
{
    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("SELECT id FROM tasks WHERE tstatus BETWEEN 1 AND 4");
    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::startAllTasks(1): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
        return;
    }

    while(qr.next())
    {
        int id_row = qr.value(0).toInt();
        int id_task = tasklist.value(id_row)%100;
        int id_proto = tasklist.value(id_row)/100;
        if(!id_task)continue;

        LoaderInterface *ldr = pluglist.value(id_proto);
        ldr->stopDownload(id_task);
    }
    updateTaskSheet();
    manageTaskQueue();
    syncTaskData();
}

void REXWindow::syncTaskData()
{
    if(tasklist.isEmpty())return;

    QList<int> keys = tasklist.keys();
    QSqlQuery qr(QSqlDatabase::database());

    for(int i = 0; i < keys.length(); i++)
    {
        int id_row = keys.value(i);
        int id_task = (tasklist.value(id_row))%100;
        int id_proto = (tasklist.value(id_row))/100;

        LoaderInterface *ldr = pluglist.value(id_proto);
        int tstatus = ldr->taskStatus(id_task);
        qr.clear();

        if(tstatus == LInterface::NO_TASK)
        {
            qr.prepare("UPDATE tasks SET tstatus=:tstatus WHERE id=:id");
            qr.bindValue("tstatus",-100);
            qr.bindValue("id",id_row);

            if(!qr.exec())
            {
                //запись в журнал ошибок
                qDebug()<<"void REXWindow::syncTaskData(3): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
            }
            else tasklist.remove(id_row);

            continue;
        }

        qint64 totalsize = ldr->totalSize(id_task);
        qint64 totalload = ldr->totalLoadedOnTask(id_task);
        QString filepath = ldr->taskFilePath(id_task);
        //QFileInfo flinfo(filepath);
        //if(flinfo.isDir()){filepath.right(1) == "/" ? filepath +="noname.html" : filepath +="/noname.html";}
        qint64 speed = ldr->downSpeed(id_task);
        model->setMetaData(id_row,"speed",speed);

        QSortFilterProxyModel *mdl = new QSortFilterProxyModel(); //определяем внутренний номер строки для обновления
        mdl->setSourceModel(model);
        mdl->setFilterKeyColumn(0);
        mdl->setFilterRole(100);
        mdl->setFilterRegExp(QString::number(id_row));
        QModelIndex index = mdl->index(0,0);
        QModelIndex downtimeId = mdl->index(0,6);
        QModelIndex speedAvgId = mdl->index(0,11);

        index = mdl->mapToSource(index);
        downtimeId = mdl->mapToSource(downtimeId);
        speedAvgId = mdl->mapToSource(speedAvgId);
        int downtime = model->data(downtimeId,100).toInt();
        int speedAvg = downtime ? (model->data(speedAvgId,100).toLongLong()*downtime + speed)/(downtime+1) : 0;
        ++downtime;
        delete(mdl);

        if(tstatus == LInterface::ERROR_TASK)
        {
            int errno = ldr->errorNo(id_task);
            QString errStr = ldr->errorString(errno);

            qr.prepare("UPDATE tasks SET totalsize=:totalsize, currentsize=:currentsize, filename=:filename, downtime=:downtime, tstatus=:tstatus, speed_avg=:speedavg,lasterror=:lasterror WHERE id=:id");
            qr.bindValue("totalsize",QString::number(totalsize));
            qr.bindValue("currentsize",QString::number(totalload));
            qr.bindValue("filename",filepath);
            qr.bindValue("downtime",downtime);
            qr.bindValue("tstatus",tstatus);
            qr.bindValue("speedavg",QString::number(speedAvg));
            qr.bindValue("lasterror",QString("%1 (%2)").arg(errStr,QString::number(errno)));
            qr.bindValue("id",id_row);

            if(!qr.exec())
            {
                //запись в журнал ошибок
                qDebug()<<"void REXWindow::syncTaskData(1): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
            }
            model->addToCache(index.row(),5,totalsize);
            model->addToCache(index.row(),6,downtime);
            model->addToCache(index.row(),4,totalload);
            model->addToCache(index.row(),3,filepath);
            model->addToCache(index.row(),9,tstatus);
            model->addToCache(index.row(),11,speedAvg);
            model->addToCache(index.row(),7,QString("%1 (%2)").arg(errStr,QString::number(errno)));

            ldr->deleteTask(id_task);
            tasklist.remove(id_row);
        }
        else
        {
            qr.prepare("UPDATE tasks SET totalsize=:totalsize, currentsize=:currentsize, filename=:filename, downtime=:downtime, tstatus=:tstatus, speed_avg=:speedavg WHERE id=:id");
            qr.bindValue("totalsize",QString::number(totalsize));
            qr.bindValue("currentsize",QString::number(totalload));
            qr.bindValue("filename",filepath);
            qr.bindValue("downtime",downtime);
            qr.bindValue("tstatus",tstatus);
            qr.bindValue("speedavg",QString::number(speedAvg));
            qr.bindValue("id",id_row);

            if(!qr.exec())
            {
                //запись в журнал ошибок
                qDebug()<<"void REXWindow::syncTaskData(2): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
            }
            model->addToCache(index.row(),5,totalsize);
            model->addToCache(index.row(),6,downtime);
            model->addToCache(index.row(),4,totalload);
            model->addToCache(index.row(),3,filepath);
            model->addToCache(index.row(),9,tstatus);
            model->addToCache(index.row(),11,speedAvg);
        }

        if(tstatus == LInterface::ON_PAUSE || tstatus == LInterface::FINISHED)
        {
            ldr->deleteTask(id_task);
            tasklist.remove(id_row);
        }
        model->updateRow(index.row());
    }
}

void REXWindow::manageTaskQueue()
{
    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("SELECT * FROM tasks WHERE tstatus=-100 ORDER BY priority DESC"); //список задач в очереди
    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::manageTaskQueue(1): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
    }

    while(tasklist.size() < max_tasks && qr.next()) //если запущено меньше задач, чем разрешено
    {
        QUrl _url(qr.value(1).toString());
        if(!plugproto.contains(_url.scheme().toLower()))
        {
            QSqlQuery qr1(QSqlDatabase::database());
            qr1.prepare("UPDATE tasks SET tstatus=:status, lasterror=:error WHERE id=:id");
            qr1.bindValue("status",LInterface::ERROR_TASK);
            qr1.bindValue("error",tr("This protocol is not supported. Check whether there is a plugin to work with the protocol and whether it is enabled."));
            qr1.bindValue("id",qr.value(0).toInt());
            if(!qr1.exec())
            {
                //запись в журнал ошибок
                qDebug()<<"void REXWindow::manageTaskQueue(2): SQL:" + qr1.executedQuery() + " Error: " + qr1.lastError().text();
            }
            continue;
        }

        int id_row = qr.value(0).toInt();
        startTaskNumber(id_row,_url,qr.value(3).toString(),qr.value(4).toLongLong());
    }

    if(!qr.next()){updateStatusBar(); return;}

    QSqlQuery qr1(QSqlDatabase::database());
    qr1.prepare("SELECT id,priority FROM tasks WHERE tstatus BETWEEN 1 AND 4 ORDER BY priority DESC"); //список выполняемых задач
    if(!qr1.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::manageTaskQueue(3): SQL:" + qr1.executedQuery() + " Error: " + qr1.lastError().text();
        return;
    }

    qr1.last();
    if(qr.value(13).toInt() <= qr1.value(13).toInt())return; //если самый высокий приоритет стоящего в очереди меньше или равен самому маленькому приоритету выполняющегося, то выходим
    qr1.first();

    int start_pos = 0;
    while(qr.next())
    {
        bool success = false;
        while(qr1.seek(start_pos))
        {
            ++start_pos;
            if(qr.value(13).toInt() > qr1.value(13).toInt())
            {
                int id_row = qr1.value(0).toInt();
                int id_task = tasklist.value(id_row)%100;
                int id_proto = tasklist.value(id_row)/100;

                LoaderInterface *ldr = pluglist.value(id_proto);

                 //останавливаем найденную задачу, удаляем её из менеджера закачек
                ldr->stopDownload(id_task);
                ldr->deleteTask(id_task);

                //запускаем новую задачу

                QUrl _url(qr.value(1).toString());
                id_row = qr.value(0).toInt();

                if(!plugproto.contains(_url.scheme().toLower())) //если протокол не поддерживается, то пропускаем эту задачу в очереди, поменяв её статус на ошибку
                {
                    QSqlQuery qr1(QSqlDatabase::database());
                    qr1.prepare("UPDATE tasks SET status=:status, lasterror=:error WHERE id=:id");
                    qr1.bindValue("status",LInterface::ERROR_TASK);
                    qr1.bindValue("error",tr("This protocol is not supported. Check whether there is a plugin to work with the protocol and whether it is enabled."));
                    qr1.bindValue("id",qr.value(0).toInt());
                    if(!qr1.exec())
                    {
                        //запись в журнал ошибок
                        qDebug()<<"void REXWindow::manageTaskQueue(2): SQL:" + qr1.executedQuery() + " Error: " + qr1.lastError().text();
                    }
                    success = true;
                    break;
                }

                startTaskNumber(id_row,_url,qr.value(3).toString(),qr.value(4).toLongLong());
                success = true;
                break;
            }
        }
        if(!success)return; //если не нашли меньших по приоритету задач то выходим
    }
    updateStatusBar();
}

void REXWindow::updateStatusBar()
{
    QItemSelectionModel *selection = ui->tableView->selectionModel();
    QLabel *urllbl, *speed, *lefttime, *lasterror, *priority, *status;
    QProgressBar *progress;
    QLabel *onplay, *onpause, *onqueue, *onerror;

    status = findChild<QLabel*>("statusIcon");
    priority = findChild<QLabel*>("priorityIcon");
    urllbl = findChild<QLabel*>("urllbl");
    speed = findChild<QLabel*>("speed");
    lefttime = findChild<QLabel*>("timeleft");
    lasterror = findChild<QLabel*>("lasterror");
    onplay = findChild<QLabel*>("onplay");
    onpause = findChild<QLabel*>("onpause");
    onqueue = findChild<QLabel*>("onqueue");
    onerror = findChild<QLabel*>("onerror");
    progress = findChild<QProgressBar*>("prgBar");

    if(!selection->hasSelection())
    {
        status->hide();
        priority->hide();
        urllbl->hide();
        progress->setValue(0);
        lasterror->hide();
    }
    else
    {
        QModelIndex curIndex = selection->currentIndex();
        int row_id = sfmodel->mapToSource(curIndex).row();

        switch(model->index(row_id,9).data(100).toInt())
        {
        case LInterface::SEND_QUERY:
        case LInterface::ACCEPT_QUERY:
        case LInterface::REDIRECT:
        case LInterface::ON_LOAD: status->setPixmap(QPixmap(":/appimages/start_16x16.png")); break;
        case LInterface::STOPPING:
        case LInterface::ON_PAUSE: status->setPixmap(QPixmap(":/appimages/pause_16x16.png")); break;
        case -100: status->setPixmap(QPixmap(":/appimages/queue_16x16.png")); break;
        default:
            status->setPixmap(QPixmap(":/appimages/error_16x16.png"));
            break;
        }
        status->setVisible(true);

        switch(model->index(row_id,13).data(100).toInt())
        {
        case 0: priority->setPixmap(QPixmap(":/appimages/start_16x16.png")); break;
        case 1: priority->setPixmap(QPixmap(":/appimages/pause_16x16.png")); break;
        case 2: priority->setPixmap(QPixmap(":/appimages/queue_16x16.png")); break;
        case 3: priority->setPixmap(QPixmap(":/appimages/queue_16x16.png")); break;
        default:
            priority->setPixmap(QPixmap(":/appimages/error_16x16.png"));
            break;
        }
        priority->setVisible(true);
        urllbl->setText(QString("<a href='%1'>%1</a>").arg(model->index(row_id,1).data(Qt::DisplayRole).toString()));
        urllbl->setVisible(true);
        progress->setMaximum(100);
        int curVal = model->index(row_id,5).data(100).toLongLong() > 0 ? ((qint64)100*model->index(row_id,4).data(100).toLongLong()/model->index(row_id,5).data(100).toLongLong()) : 0;
        progress->setValue(curVal);
        lefttime->setText(model->index(row_id,15).data(Qt::DisplayRole).toString());
        lefttime->setVisible(true);
        lasterror->setText(model->index(row_id,7).data(100).toString());
        lasterror->setVisible(true);
    }

    QSortFilterProxyModel filter;
    filter.setSourceModel(model);
    filter.setFilterRole(100);
    filter.setFilterKeyColumn(9);
    filter.setFilterRegExp(QString("%1").arg(QString::number(LInterface::ON_LOAD)));
    onplay->setText(QString::number(filter.rowCount()));
    filter.setFilterRegExp(QString("%1").arg(QString::number(LInterface::ON_PAUSE)));
    onpause->setText(QString::number(filter.rowCount()));
    filter.setFilterRegExp(QString("%1").arg(QString::number(-100)));
    onqueue->setText(QString::number(filter.rowCount()));
    filter.setFilterRegExp(QString("%1").arg(QString::number(LInterface::ERROR_TASK)));
    onerror->setText(QString::number(filter.rowCount()));
}

REXWindow::~REXWindow()
{
    delete ui;
    sched_flag = false;

    lockProcess(false);
}

bool REXWindow::event(QEvent *event)
{
    return QMainWindow::event(event);
}

void REXWindow::closeEvent(QCloseEvent *event)
 {
    if(!isHidden() && sender() != this->findChild<QAction*>("exitAct")){
        hide();
        event->ignore();
    }
    else
    {
        event->accept();
        qApp->quit();
    }
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
