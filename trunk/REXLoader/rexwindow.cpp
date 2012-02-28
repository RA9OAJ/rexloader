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
    stop_flag = false;
    max_tasks = 1;
    max_threads = 3;
    down_speed = 2048*8; // 2Mbps
    plug_state.clear();

    QList<int> sz;
    QList<int> sz1;
    QDir libdir(QApplication::applicationDirPath());
    libdir.cdUp();
    pluginDirs << libdir.absolutePath()+"/lib/rexloader/plugins" << libdir.absolutePath()+"/lib64/rexloader/plugins" << QDir::homePath()+"/.rexloader/plugins";

    downDir = QDir::homePath()+tr("/Загрузки");
    if(!QDir().exists(downDir))
        QDir().mkpath(downDir);

    sz << 800 << 150;
    sz1 << 150 << 800;
    ui->splitter->setSizes(sz);
    ui->splitter_2->setSizes(sz1);

    trayicon = new QSystemTrayIcon(this);
    trayicon->setIcon(QIcon(":/appimages/trayicon.png"));
    trayicon->show();
    movie = new QMovie(this);
    movie->setFileName(":/appimages/onload.gif");
    connect(movie,SIGNAL(updated(QRect)),this,SLOT(updateTrayIcon()));

    settDlg = new SettingsDialog(this);
    connect(settDlg,SIGNAL(newSettings()),this,SLOT(readSettings()));

    apphomedir = QDir::homePath()+"/.rexloader";

    lockProcess();
    openDataBase();

    plugmgr = new PluginManager(this);
    connect(plugmgr,SIGNAL(pluginStatus(bool)),this,SLOT(pluginStatus(bool)));
    connect(qApp,SIGNAL(aboutToQuit()),this,SLOT(prepareToQuit()));
    connect(this,SIGNAL(needExecQuery(QString)),plugmgr,SLOT(exeQuery(QString)));

    createInterface();

    QTimer::singleShot(250,this,SLOT(scheduler()));
    updateTaskSheet();
    loadSettings();
    loadPlugins();
}

void REXWindow::pluginStatus(bool stat)
{
    if(!stat)
    {
        setEnabled(false);
        int quit_ok = QMessageBox::critical(this,windowTitle()+" - "+tr("Критическая ошибка"),tr("Не найден ни один плагин.\r\n Проверьте наличие файлов плагинов в директории '.rexloader' и '/usr/{local/}lib/rexloader/plugins'."));
        if(quit_ok == QMessageBox::Ok)QTimer::singleShot(0,this,SLOT(close()));
    }
    plugmgr->restorePluginsState(plug_state);
    plugmgr->loadLocale(QLocale::system());

    scanTasksOnStart();
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
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(4);
    ui->tableView->hideColumn(7);
    ui->tableView->hideColumn(8);
    ui->tableView->hideColumn(10);
    ui->tableView->hideColumn(11);
    ui->tableView->hideColumn(13);
    ui->tableView->horizontalHeader()->moveSection(2,3);
    ui->tableView->horizontalHeader()->moveSection(14,11);
    ui->tableView->horizontalHeader()->moveSection(15,12);

    //настраиваем информационную модель
    treemodel = new TreeItemModel(this);
    treemodel->updateModel();
    ui->treeView->setModel(treemodel);
    ui->treeView->setAnimated(true);
    ui->treeView->header()->hide();
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
    ui->treeView->hideColumn(4);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setExpanded(treemodel->index(0,0),true);
    ui->treeView->setExpanded(treemodel->index(1,0),true);

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
    lay->setContentsMargins(1,1,1,1);
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
    speed->setMaximumWidth(110);
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
    connect(ui->tableView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showTableContextMenu(QPoint)));
    connect(ui->treeView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showTreeContextMenu(QPoint)));
    connect(ui->actionOpenDir,SIGNAL(triggered()),this,SLOT(openTaskDir()));
    connect(ui->actionOpenTask,SIGNAL(triggered()),this,SLOT(openTask()));
    connect(ui->actionPVeryLow,SIGNAL(triggered()),this,SLOT(setTaskPriority()));
    connect(ui->actionPLow,SIGNAL(triggered()),this,SLOT(setTaskPriority()));
    connect(ui->actionPNormal,SIGNAL(triggered()),this,SLOT(setTaskPriority()));
    connect(ui->actionPHight,SIGNAL(triggered()),this,SLOT(setTaskPriority()));
    connect(ui->actionPVeryHight,SIGNAL(triggered()),this,SLOT(setTaskPriority()));
    connect(ui->actionRedownload,SIGNAL(triggered()),this,SLOT(redownloadTask()));
    connect(ui->treeView,SIGNAL(clicked(QModelIndex)),this,SLOT(setTaskFilter(QModelIndex)));
    connect(ui->actionDelURLFiles,SIGNAL(triggered()),this,SLOT(deleteTask()));
    connect(ui->tableView,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(openTask()));
    connect(ui->tableView,SIGNAL(enterPressed()),this,SLOT(openTask()));
    connect(ui->actionAppSettings,SIGNAL(triggered()),settDlg,SLOT(show()));
    connect(ui->actionImportURL,SIGNAL(triggered()),this,SLOT(showImportFileDialog()));
    connect(ui->actionAboutQt,SIGNAL(triggered()),qApp,SLOT(aboutQt()));
    connect(ui->actionVeryLow,SIGNAL(triggered(bool)),this,SLOT(selectSpeedRate(bool)));
    connect(ui->actionLow,SIGNAL(triggered(bool)),this,SLOT(selectSpeedRate(bool)));
    connect(ui->actionNormal,SIGNAL(triggered(bool)),this,SLOT(selectSpeedRate(bool)));
    connect(ui->actionHight,SIGNAL(triggered(bool)),this,SLOT(selectSpeedRate(bool)));
    connect(ui->actionOneTask,SIGNAL(triggered()),this,SLOT(setTaskCnt()));
    connect(ui->actionTwoTasks,SIGNAL(triggered()),this,SLOT(setTaskCnt()));
    connect(ui->actionThreeTasks,SIGNAL(triggered()),this,SLOT(setTaskCnt()));
    connect(ui->actionFourTasks,SIGNAL(triggered()),this,SLOT(setTaskCnt()));
    connect(ui->actionFiveTasks,SIGNAL(triggered()),this,SLOT(setTaskCnt()));
    connect(ui->actionTaskPropert,SIGNAL(triggered()),this,SLOT(showTaskDialog()));
    connect(ui->tableView,SIGNAL(showTaskProp()),this,SLOT(showTaskDialog()));
    connect(qApp->clipboard(),SIGNAL(dataChanged()),this,SLOT(scanClipboard()));

    //кнопка-меню для выбора скорости
    spdbtn = new QToolButton(this);
    ui->actionHight->setChecked(true);
    spdbtn->setMenu(ui->menu_6);
    spdbtn->setPopupMode(QToolButton::InstantPopup);
    if(ui->actionHight->isChecked())spdbtn->setIcon(ui->actionHight->icon());
    else if(ui->actionNormal->isChecked())spdbtn->setIcon(ui->actionNormal->icon());
    else if(ui->actionLow->isChecked())spdbtn->setIcon(ui->actionLow->icon());
    else if(ui->actionVeryLow->isChecked())spdbtn->setIcon(ui->actionVeryLow->icon());
    ui->mainToolBar->addWidget(spdbtn);

    //кнопка-меню для выбора количества одновременных закачек
    taskbtn = new QToolButton(this);
    QMenu *taskmenu = new QMenu(this);
    taskmenu->setTitle(tr("Одновременные скачивания"));
    taskmenu->addAction(ui->actionOneTask);
    taskmenu->addAction(ui->actionTwoTasks);
    taskmenu->addAction(ui->actionThreeTasks);
    taskmenu->addAction(ui->actionFourTasks);
    taskmenu->addAction(ui->actionFiveTasks);
    taskbtn->setMenu(taskmenu);
    taskbtn->setPopupMode(QToolButton::InstantPopup);
    ui->mainToolBar->addWidget(taskbtn);

    //настроиваем значок в трее
    QMenu *traymenu = new QMenu(this);
    traymenu->setObjectName("traymenu");
    QAction *trayact = new QAction(this);
    trayact->setObjectName("exitAct");
    trayact->setText(tr("Выход"));
    connect(trayact,SIGNAL(triggered()),this,SLOT(close()));
    traymenu->addAction(ui->actionAdd_URL);
    traymenu->addSeparator();
    traymenu->addAction(ui->actionStartAll);
    traymenu->addAction(ui->actionStopAll);
    traymenu->addSeparator();
    traymenu->addMenu(ui->menu_6);
    traymenu->addMenu(taskmenu);
    traymenu->addSeparator();
    traymenu->addAction(ui->actionAppSettings);
    traymenu->addSeparator();
    traymenu->addAction(trayact);
    ui->menu_4->addSeparator();
    ui->menu_4->addAction(trayact);

    trayicon->setContextMenu(traymenu);
    connect(trayicon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(showHideSlot(QSystemTrayIcon::ActivationReason)));

    //настраиваем меню для таблицы задач
    QMenu *tblMenu = new QMenu(this);
    tblMenu->setObjectName("tblMenu");
    tblMenu->addAction(ui->actionOpenTask);
    tblMenu->addAction(ui->actionOpenDir);
    tblMenu->addSeparator();
    tblMenu->addAction(ui->actionStart);
    tblMenu->addAction(ui->actionStop);
    tblMenu->addSeparator();
    tblMenu->addAction(ui->actionRedownload);
    tblMenu->addSeparator();
    tblMenu->addAction(ui->actionDelURL);
    tblMenu->addAction(ui->actionDelURLFiles);
    tblMenu->addSeparator();
    tblMenu->addMenu(ui->menu_7);
    tblMenu->addSeparator();
    tblMenu->addAction(ui->actionTaskPropert);

    //Настраиваем меню для дерева категорий и фильтров
    QMenu *treeMenu = new QMenu(this);
    treeMenu->setObjectName("treeMenu");
    treeMenu->addAction(ui->actionAddCategory);
    treeMenu->addAction(ui->actionDeleteCategory);
    treeMenu->addSeparator();
    treeMenu->addAction(ui->actionCatProperties);
    connect(ui->actionDeleteCategory,SIGNAL(triggered()),this,SLOT(deleteCategory()));
    connect(ui->actionAddCategory,SIGNAL(triggered()),this,SLOT(addCategory()));
    connect(ui->actionCatProperties,SIGNAL(triggered()),this,SLOT(categorySettings()));
}

void REXWindow::showTableContextMenu(const QPoint &pos)
{
    QItemSelectionModel *selected = ui->tableView->selectionModel();
    if(!selected->selectedRows().size())
    {
        setEnabledTaskMenu(false);
        return; //если ничего не выделено
    }
    setEnabledTaskMenu(true);

    QMenu *mnu = findChild<QMenu*>("tblMenu");
    if(mnu)mnu->popup(QCursor::pos());
}

void REXWindow::setTaskFilter(const QModelIndex &index)
{
    QModelIndex mindex = treemodel->index(index.row(),1,index.parent());
    int id_cat = treemodel->data(mindex,100).toInt();
    if(id_cat == 1 || id_cat == -1)
    {
        sfmodel->setFilterRegExp("");
        return;
    }

    int key_column = 10;
    int model_column = 1;
    int subcat_cnt = treemodel->rowCount(index);
    QString filter = "";

    if(id_cat < 0)
    {
        key_column = 9;
        model_column = 3;
        mindex = treemodel->index(index.row(),3,index.parent());
        filter = QString("(^%1$)").arg(QString::number(treemodel->data(mindex,100).toInt()));
    }
    else filter = QString("(^%1$)").arg(QString::number(id_cat));
    if(subcat_cnt)
    {
        QModelIndex child;
        for(int i = 0; i < subcat_cnt; ++i)
        {
            child = treemodel->index(i,model_column,index);
            filter += QString("|(^%1$)").arg(QString::number(treemodel->data(child,100).toInt()));
        }
    }
    sfmodel->setFilterRole(100);
    sfmodel->setFilterKeyColumn(key_column);
    sfmodel->setFilterRegExp(filter);
}

void REXWindow::openTask()
{
    QItemSelectionModel *selected = ui->tableView->selectionModel();
    if(!selected->selectedRows().size())return; //если ничего не выделено
    QStringList pathlist;
    QList<QModelIndex> rows = selected->selectedRows(3);
    for(int i = 0; i < rows.size(); ++i)
    {
        QString path = rows.value(i).data(100).toString();
        if(pathlist.contains(path))continue;
        pathlist.append(path);
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void REXWindow::openTaskDir()
{
    QItemSelectionModel *selected = ui->tableView->selectionModel();
    if(!selected->selectedRows().size())return; //если ничего не выделено
    QStringList pathlist;
    QList<QModelIndex> rows = selected->selectedRows(3);
    for(int i = 0; i < rows.size(); ++i)
    {
        QString path = rows.value(i).data(100).toString();
        if(pathlist.contains(path))continue;
        pathlist.append(path);
        QFileInfo flinfo(path);
        if(!flinfo.isDir())path = flinfo.absolutePath();
        QDesktopServices::openUrl(QUrl::fromLocalFile(path));
    }
}

void REXWindow::setEnabledTaskMenu(bool stat)
{
    if(!stat)
    {
        ui->menu_7->setEnabled(false);
        ui->actionOpenDir->setEnabled(false);
        ui->actionOpenTask->setEnabled(false);
        ui->actionTaskPropert->setEnabled(false);
        ui->actionRedownload->setEnabled(false);
        return;
    }

    if(ui->tableView->selectionModel()->selectedRows().size() < 2 && model->index(ui->tableView->currentIndex().row(),9).data(100) == LInterface::FINISHED)
        ui->menu_7->setEnabled(false);
    else
        ui->menu_7->setEnabled(true);
    if(ui->tableView->selectionModel()->selectedRows().size() < 2 && model->index(ui->tableView->currentIndex().row(),9).data(100) != LInterface::FINISHED)
        ui->actionRedownload->setEnabled(false);
    else
        ui->actionRedownload->setEnabled(true);
    ui->actionOpenDir->setEnabled(true);
    ui->actionOpenTask->setEnabled(true);
    ui->actionTaskPropert->setEnabled(true);
}

void REXWindow::setTaskPriority()
{
    QItemSelectionModel *selected = ui->tableView->selectionModel();
    if(!selected->selectedRows().size())return; //если ничего не выделено
    QAction *act = qobject_cast<QAction*>(sender());
    if(!act)return;
    if(!act->isChecked())act->setChecked(true);
    QList<QModelIndex> rows = selected->selectedRows(0);
    QString where;
    int newprior = 0;

    if(act == ui->actionPLow)
    {
        newprior = 1;
        ui->actionPVeryLow->setChecked(false);
        ui->actionPNormal->setChecked(false);
        ui->actionPHight->setChecked(false);
        ui->actionPVeryHight->setChecked(false);
    }
    else if(act == ui->actionPNormal)
    {
        newprior = 2;
        ui->actionPVeryLow->setChecked(false);
        ui->actionPLow->setChecked(false);
        ui->actionPHight->setChecked(false);
        ui->actionPVeryHight->setChecked(false);
    }
    else if(act == ui->actionPHight)
    {
        newprior = 3;
        ui->actionPVeryLow->setChecked(false);
        ui->actionPLow->setChecked(false);
        ui->actionPNormal->setChecked(false);
        ui->actionPVeryHight->setChecked(false);
    }
    else if(act == ui->actionPVeryHight)
    {
        newprior = 4;
        ui->actionPVeryLow->setChecked(false);
        ui->actionPLow->setChecked(false);
        ui->actionPNormal->setChecked(false);
        ui->actionPHight->setChecked(false);
    }
    else
    {
        ui->actionPLow->setChecked(false);
        ui->actionPNormal->setChecked(false);
        ui->actionPHight->setChecked(false);
        ui->actionPVeryHight->setChecked(false);
    }


    for(int i = 0; i < rows.size(); ++i)
    {
        if(!i)
        {
            where = QString("id=%1").arg(QString::number(rows.value(i).data(100).toInt()));
            continue;
        }
        where += QString(" OR id=%1").arg(QString::number(rows.value(i).data(100).toInt()));
    }
    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("UPDATE tasks SET priority=:priority WHERE "+where);
    qr.bindValue("priority",newprior);
    if(!qr.exec())
    {
        //записываем ошибку в журнал
        qDebug()<<"void REXWindow::setTaskPriority(1)" + qr.executedQuery() + " Error:" + qr.lastError().text();
    }
    updateTaskSheet();
}

void REXWindow::showNotice(const QString &title, const QString &text, int type)
{

}

void REXWindow::saveSettings()
{
    QSettings settings(apphomedir+"/window.ini", QSettings::IniFormat,this);
    settings.beginGroup("Main Window");
    settings.setValue("WindowGeometry",saveGeometry());
    settings.setValue("WindowState",saveState());
    settings.endGroup();
    settings.beginGroup("Windgets Interface");
    settings.setValue("TasksTable",ui->tableView->horizontalHeader()->saveState());
    settings.setValue("WindowHSplitter", ui->splitter_2->saveState());
    settings.setValue("WindowVSplitter", ui->splitter->saveState());
    settings.setValue("S_hight",ui->actionHight->isChecked());
    settings.setValue("S_normal",ui->actionNormal->isChecked());
    settings.setValue("S_low",ui->actionLow->isChecked());
    settings.setValue("S_verylow",ui->actionVeryLow->isChecked());
    settings.endGroup();
    settings.beginWriteArray("TreeViewNodes");
    QList<QModelIndex> list = treemodel->parentsInTree();
    for(int i = 0; i < list.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("State",ui->treeView->isExpanded(list.value(i)));
    }
    settings.endArray();
    settings.beginWriteArray("Application Settings");
    QList<QString> keys = settDlg->keys();
    for(int i = 0; i < keys.size(); ++i)
    {
        settings.setArrayIndex(i);
        settings.setValue("Key",keys.value(i));
        settings.setValue("Value",settDlg->value(keys.value(i)));
    }
    settings.endArray();
    settings.beginGroup("State Plugins");
    settings.setValue("EnablePlugins",plugmgr->pluginsState());
    settings.endGroup();
    settings.sync();
}

void REXWindow::loadSettings()
{
    QSettings settings(apphomedir+"/window.ini", QSettings::IniFormat,this);
    settings.beginGroup("Main Window");
    restoreGeometry(settings.value("WindowGeometry").toByteArray());
    restoreState(settings.value("WindowState").toByteArray());
    settings.endGroup();

    settings.beginGroup("Windgets Interface");
    ui->tableView->horizontalHeader()->restoreState(settings.value("TasksTable").toByteArray());
    ui->splitter_2->restoreState(settings.value("WindowHSplitter").toByteArray());
    ui->splitter->restoreState(settings.value("WindowVSplitter").toByteArray());
    ui->actionHight->setChecked(settings.value("S_hight",true).toBool());
    if(ui->actionHight->isChecked()) movie->setSpeed(100);
    ui->actionNormal->setChecked(settings.value("S_normal",false).toBool());
    if(ui->actionNormal->isChecked()) movie->setSpeed(75);
    ui->actionLow->setChecked(settings.value("S_low",false).toBool());
    if(ui->actionLow->isChecked()) movie->setSpeed(50);
    ui->actionVeryLow->setChecked(settings.value("S_verylow",false).toBool());
    if(ui->actionVeryLow->isChecked()) movie->setSpeed(25);
    settings.endGroup();
    settings.beginWriteArray("TreeViewNodes");
    QList<QModelIndex> list = treemodel->parentsInTree();
    for(int i = 0; i < list.size(); ++i)
    {
        settings.setArrayIndex(i);
        ui->treeView->setExpanded(list.value(i),settings.value("State").toBool());
    }
    settings.endArray();
    int cnt = settings.beginReadArray("Application Settings");
    for(int i = 0; i < cnt; ++i)
    {
        settings.setArrayIndex(i);
        QString key;
        QVariant value;
        key = settings.value("Key").toString();
        value = settings.value("Value");
        settDlg->setSettingAttribute(key,value);
    }
    settDlg->updateInterface();
    settings.endArray();

    if(settDlg->value("start_minimized").toBool())
    {
        preStat = windowState();
        setWindowState(Qt::WindowMinimized);
        QTimer::singleShot(0,this,SLOT(close()));
    }

    settings.beginGroup("State Plugins");
    plug_state = settings.value("EnablePlugins").toByteArray();
    settings.endGroup();
}

void REXWindow::scanClipboard()
{
    if(!settDlg->value("scan_clipboard").toBool())return;

    const QClipboard *clipbrd = QApplication::clipboard();
    QUrl url = QUrl::fromEncoded(clipbrd->text().toUtf8());
    if(url.isValid() && plugproto.contains(url.scheme().toLower()) && url.toString() != clip_last)
    {
        clip_last = url.toString();
        AddTaskDialog *dlg = new AddTaskDialog(downDir, 0);
        connect(dlg,SIGNAL(addedNewTask()),this,SLOT(updateTaskSheet()));
        connect(dlg,SIGNAL(addedNewTask()),ui->tableView,SLOT(scrollToBottom()));
        dlg->setValidProtocols(plugproto);
        dlg->setNewUrl(url.toString());
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
        int quit_ok = QMessageBox::critical(this,windowTitle()+" - "+tr("Критическая ошибка"),tr("Невозможно открыть файл базы данных.\r\n Это критическая ошибка, приложение будет закрыто.\r\n Проверьте свои права доступа к директории '.rexloader'."));
        if(quit_ok == QMessageBox::Ok)QTimer::singleShot(0,this,SLOT(close()));
    }

    QSqlQuery qr;
    qr.prepare("UPDATE tasks SET tstatus=:newstatus WHERE tstatus=:tstatus;");
    qr.bindValue("newstatus",-100);
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
            if(QFile::exists(qr.value(1).toString()))
            {
                //тут считываем данные о загрузке из недокачанного файла
                QString _path = qr.value(1).toString();
                QFileInfo flinfo(_path);
                switch(1)
                {
                case 1:
                {
                    QFile fl(_path);
                    if(!fl.open(QFile::ReadOnly)) break;
                    QDataStream in(&fl);

                    qint64 spos = 0;

                    if(!fl.seek(flinfo.size()-8)){fl.close();break;}
                    in >> spos;
                    if(!fl.seek(spos)){fl.close();break;}

                    QString header;
                    header = fl.readLine(3);
                    if(header != "\r\n"){fl.close();break;}
                    header = fl.readLine(1024);
                    if(header.indexOf("RExLoader")!= 0){fl.close();break;}
                    QString fversion = header.split(" ").value(1);
                    if(fversion != "0.1a.1\r\n"){fl.close();break;}

                    int length = 0;
                    in >> length;
                    QByteArray buffer;
                    buffer.resize(length);
                    in.readRawData(buffer.data(),length); //считываем URL

                    QUrl newurl = QUrl::fromEncoded(buffer, QUrl::StrictMode);
                    if(!newurl.isValid()){fl.close();break;}

                    length = 0;
                    in >> length;
                    buffer.resize(length);
                    in.readRawData(buffer.data(),length); //считываем реферера
                    QString ref = buffer;

                    length = 0;
                    in >> length;
                    buffer.resize(length);
                    in.readRawData(buffer.data(),length); //считываем MIME
                    QString mime = buffer;
                    qint64 tsize = 0;
                    in >> tsize; //считываем общий размер задания

                    qint64 sum = 0;
                    for(int i=1; i<12; i+=2)
                    {
                        qint64 tmp = 0;
                        in >> tmp; //считывание карты секций
                        sum += tmp;
                    }

                    length = 0;
                    in >> length;
                    buffer.resize(length);
                    in.readRawData(buffer.data(),length); //считываем дату модификации
                    fl.close();

                    dlg = new AddTaskDialog(downDir, this);
                    connect(dlg,SIGNAL(addedNewTask()),this,SLOT(updateTaskSheet()));
                    connect(dlg,SIGNAL(addedNewTask()),ui->tableView,SLOT(scrollToBottom()));
                    dlg->setValidProtocols(plugproto);
                    dlg->setNewUrl(QString(newurl.toString()));
                    dlg->setAdditionalInfo(_path,sum,tsize,mime);
                    dlg->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
                    dlg->show();
                }
                default: break;
                }
            }
            else if(plugproto.contains(url.scheme().toLower()))
            {
                dlg = new AddTaskDialog(downDir, this);
                connect(dlg,SIGNAL(addedNewTask()),this,SLOT(updateTaskSheet()));
                dlg->setValidProtocols(plugproto);
                dlg->setNewUrl(qr.value(1).toString());
                dlg->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
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

void REXWindow::loadPlugins()
{
    QString dbfile = QDir::homePath()+"/.rexloader/tasks.db";
    plugmgr->setDatabaseFile(dbfile);

    plugmgr->setDefaultSettings(max_tasks, max_threads, down_speed);
    plugmgr->setPlugDir(pluginDirs);
    plugmgr->setPlugLists(&plugfiles, &pluglist, &plugproto);
    plugmgr->start();
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

void REXWindow::scheduler()
{
    if(!sched_flag)return;
    lockProcess(true);

    scanNewTaskQueue();
    syncTaskData();
    manageTaskQueue();

    QTimer::singleShot(1000,this,SLOT(scheduler()));
}

void REXWindow::updateTaskSheet()
{
    model->silentUpdateModel();
    model->clearCache();
}

void REXWindow::startTrayIconAnimaion()
{
    if(movie->state() == QMovie::Running || !settDlg->value("animate_tray").toBool())return;
    movie->start();
}

void REXWindow::stopTrayIconAnimation()
{
    if(movie->state() == QMovie::NotRunning)return;
    movie->stop();
    trayicon->setIcon(QIcon(":/appimages/trayicon.png"));
}

void REXWindow::updateTrayIcon()
{
    if(!settDlg->value("animate_tray").toBool())
    {
        stopTrayIconAnimation();
        return;
    }
    trayicon->setIcon(QIcon(movie->currentPixmap()));
}

void REXWindow::showAddTaskDialog()
{
    AddTaskDialog *dlg = new AddTaskDialog(downDir, this);
    connect(dlg,SIGNAL(addedNewTask()),this,SLOT(updateTaskSheet()));
    connect(dlg,SIGNAL(addedNewTask()),ui->tableView,SLOT(scrollToBottom()));
    dlg->setValidProtocols(plugproto);
    if(!isVisible()) dlg->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    dlg->show();
}

void REXWindow::showHideSlot(QSystemTrayIcon::ActivationReason type)
{
    if(type == QSystemTrayIcon::DoubleClick)
    {
        if(isVisible())
        {
            setHidden(true);
            settDlg->setWindowFlags(Qt::Window);
            preStat = windowState();
        }
        else
        {
            setHidden(false);
            settDlg->setWindowFlags(Qt::Dialog);
            setWindowState(preStat);
            activateWindow();
        }
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
        if(QFile::exists(filename) && flinfo.isFile()) //если локальный файл существует
        {
            id_task = pluglist.value(id_proto)->loadTaskFile(filename); // id задачи
            if(id_task) //если плагин удачно прочитал метаданные и добавил задачу
            {
                tasklist.insert(id_row, id_task + id_proto*100);
                calculateSpeed();

                setProxy(id_task, id_proto);

                plugmgr->startDownload(id_task + id_proto*100);
                updateTaskSheet();
                return;
            }
            else
            {
                QFile::remove(filename);
                //тут запись в журнал ошибок или запрос на счет того, желает ли пользователь снова закачать файл с самогог начала

            }
        }
    }

    //дальше идет код, который добавляет новую закачку на основе URL в очередь плагина
    id_task = pluglist.value(id_proto)->addTask(url);

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
    updateTaskSheet();
    QString fldir = (flinfo.isDir() || flinfo.fileName() == "noname.html") ? flinfo.absolutePath():flinfo.absoluteFilePath();
    flinfo.setFile(fldir);
    if(!flinfo.isDir() && fldir.right(5) != ".rldr") fldir += "." + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".rldr";
    tasklist.insert(id_row, id_task + id_proto*100);
    pluglist.value(id_proto)->setTaskFilePath(id_task,fldir);
    calculateSpeed();
    //pluglist.value(id_proto)->startDownload(id_task);
    setProxy(id_task, id_proto);
    plugmgr->startDownload(id_task + id_proto*100);
}

void REXWindow::deleteTask()
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())return; //если ничего невыделено, то выходим
    bool del_file = false; //удалять ли результаты скачивания на локальном диске
    if(qobject_cast<QAction*>(sender()) == ui->actionDelURLFiles) del_file = true;

    if(select->selectedRows().length() > 1) //если выделено более 1 строки
    {
        EMessageBox dlg(this);
        dlg.setWindowTitle(tr("Что делать?"));
        dlg.setIcon(EMessageBox::Warning);
        dlg.setText(tr("Выбрано более одного задания."));
        dlg.setInformativeText("Чтобы подтвердить удаление нескольких заданий нажмите <b>\"Ok\"</b> или <b>\"Отмена\"</b> для отмены удаления.");
        dlg.setStandardButtons(EMessageBox::Ok | EMessageBox::Cancel);
        dlg.setDefaultButton(EMessageBox::Cancel);

        int result = dlg.exec();
        if(result == EMessageBox::Cancel)return;
    }

    stopTask(); //останавливаем задания, попавшие в выделение

    QSqlQuery qr(QSqlDatabase::database());

    QMap<int,int> cat_map;
    QString where;
    for(int i=0; i < select->selectedRows().length(); i++)
    {
        int row_id = select->selectedRows().value(i).data(Qt::DisplayRole).toInt();
        cat_map.insert(select->selectedRows().value(i).data(Qt::DisplayRole).toInt(),0);
        QString flname = select->selectedRows(3).value(i).data(100).toString();
        int task_id = tasklist.value(row_id)%100;
        LoaderInterface *ldr = pluglist.value(tasklist.value(row_id)/100);

        if(!i)
            where += QString(" id=%1").arg(QString::number(row_id));
        else
            where += QString(" OR id=%1").arg(QString::number(row_id));

        if(del_file)
        {
            QFileInfo fl(flname);
            if(!fl.isFile())continue;
            QFile::remove(flname);
        }

        if(dlglist.contains(row_id))
        {
            dlglist.value(row_id)->close();
            dlglist.remove(row_id);
        }

        if(ldr)ldr->deleteTask(task_id);
        tasklist.remove(row_id);
    }

    qr.prepare("DELETE FROM tasks WHERE"+where);

    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::deleteTask(): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
    }

    model->updateModel(); //обновляем таблицу задач
    model->clearCache();

    foreach(int cur_row, cat_map.keys()) //обновляем счетчики в строках категорий
        treemodel->updateRow(treemodel->indexById(cur_row));

    manageTaskQueue();
}

void REXWindow::startTask()
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())return; //если ничего невыделено, то выходим

    QString where;
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
        if(!i || where.isEmpty()) where = QString("id=%1").arg(QString::number(id_row));
        else where += QString(" OR id=%1").arg(QString::number(id_row));
    }
    if(where.isEmpty())return;

    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("UPDATE tasks SET tstatus=-100, lasterror='' WHERE " + where);

    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::startTask(1): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
    }
    updateTaskSheet();

    for(int i=0; i < select->selectedRows().length(); i++)
    {
        QModelIndex index = sfmodel->mapToSource(select->selectedRows(9).value(i));
        //int tstatus = select->selectedRows(9).value(i).data(100).toInt(); //статус в базе данных
        model->updateRow(index.row());
    }
    manageTaskQueue();
    syncTaskData();
}

void REXWindow::startTask(int id)
{
    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("UPDATE tasks SET tstatus=-100, lasterror='' WHERE id=:id AND tstatus <> :tstatus");
    qr.bindValue("id",id);
    qr.bindValue("tstatus",(int)LInterface::FINISHED);

    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::startTask(2): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
        return;
    }
    updateTaskSheet();

    QSortFilterProxyModel smodel;
    smodel.setSourceModel(model);
    smodel.setFilterRole(100);
    smodel.setFilterKeyColumn(0);
    smodel.setFilterFixedString(QString::number(id));
    if(!smodel.rowCount()) return;
    QModelIndex idx = smodel.mapToSource(smodel.index(0,0));
    model->updateRow(idx.row());

    manageTaskQueue();
    syncTaskData();
}

void REXWindow::startAllTasks()
{
    /*QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("UPDATE tasks SET tstatus=-100, lasterror='' WHERE tstatus IN (-100,0)");
    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::startAllTasks(1): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
        return;
    }*/
    emit needExecQuery("UPDATE tasks SET tstatus=-100, lasterror='' WHERE tstatus IN (-100,0)");

    QSortFilterProxyModel fltr;
    fltr.setSourceModel(model);
    fltr.setFilterRole(100);
    fltr.setFilterKeyColumn(9);
    fltr.setFilterFixedString("0");
    for(int i = 0; i < fltr.rowCount(); ++i)
    {
        QModelIndex index = fltr.mapToSource(fltr.index(i,0));
        model->addToCache(index.row(),9,-100);
        model->addToCache(index.row(),7,"");
        model->updateRow(index.row());
    }

    manageTaskQueue();
    syncTaskData();
}

void REXWindow::redownloadTask()
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())return; //если ничего не выделено, то выходим
    QSqlQuery qr(QSqlDatabase::database());
    QString where;

    for(int i=0; i < select->selectedRows().length(); i++)
    {
        if(select->selectedRows(9).value(i).data(100).toInt() != LInterface::FINISHED)continue;
        if(where.isEmpty())
        {
            where = QString("id=%1").arg(QString::number(select->selectedRows(0).value(i).data(100).toInt()));
            continue;
        }
        where += QString(" OR id=%1").arg(QString::number(select->selectedRows(0).value(i).data(100).toInt()));
    }
    qr.prepare("UPDATE tasks SET tstatus=-100, currentsize=NULL, totalsize=NULL, speed_avg=NULL, downtime=NULL, datecreate=:datecreate WHERE "+where);
    qr.bindValue("datecreate",QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss"));
    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::redownloadTask(1): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
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

    QString where;
    for(int i=0; i < select->selectedRows().length(); i++)
    {
        //QString filepath = select->selectedRows(3).value(i).data(100).toString(); //путь к локальному файлу закачки
        QUrl _url(select->selectedRows(1).value(i).data(100).toString()); //URL закачки
        int id_row = select->selectedRows(0).value(i).data(100).toInt(); // id записи в базе данных
        int tstatus = select->selectedRows(9).value(i).data(100).toInt(); //статус в базе данных

        switch(tstatus)
        {
        case LInterface::ON_LOAD:
        case LInterface::SEND_QUERY:
        case -100:
        case LInterface::ACCEPT_QUERY: break;
        default: continue;
        }

        int id_proto = plugproto.value(_url.scheme().toLower()); // id плагина с соответствующим протоколом
        //int id_task = tasklist.value(id_row)%100; // id задачи

        if(pluglist.contains(id_proto)) plugmgr->stopDownload(tasklist.value(id_row));//pluglist.value(id_proto)->stopDownload(id_task);
        if(!i || where.isEmpty())where = QString("id=%1").arg(QString::number(id_row));
        else where += QString(" OR id=%1").arg(QString::number(id_row));
    }

    if(!where.isEmpty())
    {
        QSqlQuery qr(QSqlDatabase::database());
        qr.prepare("UPDATE tasks SET tstatus=0, lasterror='' WHERE " + where);

        if(!qr.exec())
        {
            //запись в журнал ошибок
            qDebug()<<"void REXWindow::stopTask(1): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
        }
    }

    updateTaskSheet();
    for(int i=0; i < select->selectedRows().length(); i++)
    {
        QModelIndex index = sfmodel->mapToSource(select->selectedRows(9).value(i));
        model->updateRow(index.row());
    }
    manageTaskQueue();
    syncTaskData();
}

void REXWindow::stopTask(int id)
{
    int task_id = tasklist.value(id,0);
    if(!task_id) return;

    int id_proto = task_id/100;
    if(pluglist.contains(id_proto)) plugmgr->stopDownload(task_id);

    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("UPDATE tasks SET tstatus=0, lasterror='' WHERE id=:id");
    qr.bindValue("id",id);

    if(!qr.exec())
    {
        //запись в журнал ошибок
        qDebug()<<"void REXWindow::stopTask(2): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
        return;
    }

    updateTaskSheet();

    QSortFilterProxyModel smodel;
    smodel.setSourceModel(model);
    smodel.setFilterRole(100);
    smodel.setFilterKeyColumn(0);
    smodel.setFilterFixedString(QString::number(id));
    if(!smodel.rowCount()) return;
    QModelIndex idx = smodel.mapToSource(smodel.index(0,0));
    model->updateRow(idx.row());

    manageTaskQueue();
    syncTaskData();
}

void REXWindow::stopAllTasks()
{
    QSortFilterProxyModel select;
    select.setSourceModel(model);
    select.setFilterKeyColumn(9);
    select.setFilterRole(100);
    select.setFilterRegExp("^[1234]$");

    for(int i = 0; i < select.rowCount(); ++i)
    {
        QModelIndex idx = select.index(i,0);
        int id_row = select.data(idx,100).toInt();
        int id_task = tasklist.value(id_row)%100;
        if(!id_task)continue;

        plugmgr->stopDownload(tasklist.value(id_row));
    }

    if(!stop_flag) //если программа продолжает штатную работу
    {
        emit needExecQuery("UPDATE tasks SET tstatus=0 WHERE tstatus=-100");

        QSortFilterProxyModel fltr;
        fltr.setSourceModel(model);
        fltr.setFilterRole(100);
        fltr.setFilterKeyColumn(9);
        fltr.setFilterFixedString("-100");
        for(int i = 0; i < fltr.rowCount(); ++i)
        {
            QModelIndex index = fltr.mapToSource(fltr.index(i,0));
            model->addToCache(index.row(),9,0);
            model->updateRow(index.row());
        }
        manageTaskQueue();
    }
    else //если программа завершает свою работу
    {
        QSqlQuery qr(QSqlDatabase::database());

        qr.clear();
        qr.prepare("UPDATE tasks SET tstatus=0 WHERE tstatus BETWEEN 1 AND 4 OR tstatus=-100");
        if(!qr.exec())
        {
            //запись в журнал ошибок
            qDebug()<<"void REXWindow::startAllTasks(3): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
            return;
        }
    }
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
        if(!ldr)continue;
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
        QFileInfo flinfo(filepath);
        if(flinfo.isDir())
        {
            QSortFilterProxyModel fmodel;
            fmodel.setSourceModel(model);
            fmodel.setFilterKeyColumn(0);
            fmodel.setFilterRole(100);
            fmodel.setFilterFixedString(QString::number(id_row));
            filepath = fmodel.data(fmodel.index(0,3)).toString();
        }
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
            int errno_ = ldr->errorNo(id_task);
            QString errStr;

            switch(errno_)
            {
            case LInterface::FILE_NOT_FOUND: errStr = tr("Файл не найден.");break;
            case LInterface::FILE_DATETIME_ERROR: errStr = tr("Файл на стороне сервера был изменён.");break;
            case LInterface::FILE_SIZE_ERROR: errStr = tr("Размер файла на сервере отличается от размера задания.");break;
            case LInterface::FILE_CREATE_ERROR: errStr = tr("Невозможно создать файл на локальном диске.");break;
            case LInterface::FILE_WRITE_ERROR: errStr = tr("Невозможно записать в локальный файл.");break;
            case LInterface::FILE_READ_ERROR:errStr = tr("Невозможно прочитать локальный файл.");break;
            case LInterface::HOST_NOT_FOUND: errStr = tr("Удаленнй сервер не найден.");break;
            case LInterface::CONNECT_ERROR: errStr = tr("Ошибка подключения к удалённому серверу.");break;
            case LInterface::CONNECT_LOST: errStr = tr("Подключение к удаленному серверу потеряно.");break;
            case LInterface::SERVER_REJECT_QUERY: errStr = tr("Сервер отклонил запрос на соединение.");break;
            case LInterface::PROXY_NOT_FOUND: errStr = tr("Прокси не найден.");break;
            case LInterface::PROXY_AUTH_ERROR: errStr = tr("Не удалось пройти аутентификацию на прокси.");break;
            case LInterface::PROXY_ERROR: errStr = tr("Ошибка протокола прокси.");break;
            case LInterface::PROXY_CONNECT_CLOSE: errStr = tr("Прокси неожиданно разорвал соединение.");break;
            case LInterface::PROXY_CONNECT_REFUSED: errStr = tr("Прокси отверг попытку подключения.");break;
            case LInterface::PROXY_TIMEOUT: errStr = tr("Таймаут прокси.");break;
            case LInterface::ERRORS_MAX_COUNT: errStr = tr("Достигнуто максимальное количество ошибок.");break;
            default:
                errStr = ldr->errorString(errno_);
            }

            QString query = QString("UPDATE tasks SET totalsize='%1', currentsize='%2', filename='%3', downtime='%4', tstatus='%5', speed_avg='%6',lasterror='%7' WHERE id=%8").arg(
                        QString::number(totalsize),
                        QString::number(totalload),
                        filepath,
                        QString::number(downtime),
                        QString::number(tstatus),
                        QString::number(speedAvg),
                        tr("%1 (Код ошибки: %2)").arg(errStr,QString::number(errno_)),
                        QString::number(id_row));
            emit needExecQuery(query);

            model->addToCache(index.row(),5,totalsize);
            model->addToCache(index.row(),6,downtime);
            model->addToCache(index.row(),4,totalload);
            model->addToCache(index.row(),3,filepath);
            model->addToCache(index.row(),9,tstatus);
            model->addToCache(index.row(),11,speedAvg);
            model->addToCache(index.row(),7,QString("%1 (%2)").arg(errStr,QString::number(errno_)));

            ldr->deleteTask(id_task);
            tasklist.remove(id_row);
            calculateSpeed();
        }
        else
        {
            if(tstatus == LInterface::FINISHED)
            {
                QString newFilename = filepath.right(5) == ".rldr" ? filepath.left(filepath.size()-20) : filepath;
                QFile fl(filepath);
                if(fl.exists(newFilename))
                {
                    EMessageBox *question = new EMessageBox(this);
                    QPushButton *btn1;
                    question->setIcon(EMessageBox::Question);
                    question->addButton(tr("Заменить"),EMessageBox::ApplyRole);
                    btn1 = question->addButton(tr("Переименовать"),EMessageBox::RejectRole);
                    question->setDefaultButton(btn1);
                    question->setText(tr("Файл <b>%1</b> уже существет.").arg(newFilename));
                    question->setInformativeText(tr("Для замены существующего файла нажмите \"Заменить\". Для переименования нажмите \"Переименовать\"."));
                    question->setActionType(EMessageBox::AT_RENAME);
                    connect(question,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(acceptQAction(QAbstractButton*)));

                    int index = newFilename.lastIndexOf(".");
                    QString reFilename;
                    if(index < 1) reFilename = newFilename + QDateTime::currentDateTime().toString("_dd-MM-yyyy_hh-mm-ss-z");
                    else
                    {
                        reFilename = newFilename;
                        reFilename = reFilename.insert(index,QDateTime::currentDateTime().toString("_dd-MM-yyyy_hh-mm-ss-z"));
                    }
                    QString params = QString("curname:%1\r\nnewname:%2\r\nrename:%3\r\nid:%4").arg(filepath,newFilename,reFilename,QString::number(id_row));
                    question->setParams(params);
                    question->setModal(false);
                    if(!isVisible())question->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
                    question->show();
                }
                else
                {
                    fl.rename(newFilename);
                    filepath = newFilename;
                }
            }
            QString query = QString("UPDATE tasks SET totalsize='%1', currentsize='%2', filename='%3', downtime=%4, tstatus=%5, speed_avg='%6' WHERE id=%7").arg(
                        QString::number(totalsize),
                        QString::number(totalload),
                        filepath,
                        QString::number(downtime),
                        QString::number(tstatus),
                        QString::number(speedAvg),
                        QString::number(id_row));
            emit needExecQuery(query);

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
            calculateSpeed();
        }
        model->addToUpdateRowQueue(index.row());
        QTimer::singleShot(0,model,SLOT(updateRow()));
    }

    QSortFilterProxyModel select;
    select.setSourceModel(model);
    select.setFilterKeyColumn(9);
    select.setFilterRole(100);
    select.setFilterRegExp("(^-100$)|(^-2$)|(^0$)");
    if(!tasklist.size() && !select.rowCount())
        trayicon->showMessage(tr("REXLoader"),tr("Все задания завершены."));
}

void REXWindow::manageTaskQueue()
{
    if(stop_flag)return;

    QSortFilterProxyModel select;
    select.setSourceModel(model);
    select.setDynamicSortFilter(true);
    select.setFilterRole(100);
    select.setFilterKeyColumn(9);
    select.setFilterFixedString("-100");
    select.setSortRole(100);
    select.sort(13,Qt::DescendingOrder);

    int i = 0;
    for(; tasklist.size() < max_tasks && i < select.rowCount(); ++i)
    {
        int id_row = select.data(select.index(i,0),100).toInt();
        QUrl _url = QUrl::fromEncoded(select.data(select.index(i,1),100).toByteArray());
        if(!plugproto.contains(_url.scheme().toLower()))
        {
            QString err = tr("Этот протокол не поддерживается. Проверьте наличие соответствующего плагина и его состояние.");
            QString query = QString("UPDATE tasks SET tstatus=%1, lasterror='%2' WHERE id=%3").arg(
                        QString::number((int)LInterface::ERROR_TASK),
                        err,
                        QString::number(id_row));
            emit needExecQuery(query);

            QModelIndex srcIdx = select.mapToSource(select.index(i,9));
            model->addToCache(srcIdx.row(),srcIdx.column(),(int)LInterface::ERROR_TASK);
            srcIdx = model->index(srcIdx.row(),7);
            model->addToCache(srcIdx.row(),srcIdx.column(),err);
            model->updateRow(srcIdx.row());
            continue;
        }

        startTaskNumber(id_row,_url,select.data(select.index(i,3),100).toString(),select.data(select.index(i,4),100).toLongLong());
    }

    if(i >= select.rowCount())
    {
        if(tasklist.size() > 0)startTrayIconAnimaion();
        else stopTrayIconAnimation();
        QTimer::singleShot(0,this,SLOT(updateStatusBar()));
        return;
    }

    QSortFilterProxyModel select1;
    select1.setSourceModel(model);
    select1.setFilterRole(100);
    select1.setFilterKeyColumn(9);
    select1.setFilterRegExp("^[1234]$");
    select1.setSortRole(100);
    select1.sort(13,Qt::AscendingOrder);

    if(!select.rowCount())return;

    for(int y = 0; i < select.rowCount() && y < select1.rowCount(); ++i, ++y)
    {
        if(select.data(select.index(i,13),100).toInt() <= select1.data(select1.index(y,13),100).toInt()) break; //если самый высокий приоритет стоящего в очереди меньше или равен самому маленькому приоритету выполняющегося, то выходим
        else
        {
            int id_row = select1.data(select1.index(y,0),100).toInt();
            int id_task = tasklist.value(id_row)%100;
            int id_proto = tasklist.value(id_row)/100;

            LoaderInterface *ldr = pluglist.value(id_proto);

            //останавливаем найденную задачу, удаляем её из менеджера закачек
            ldr->stopDownload(id_task);
            QString query = QString("UPDATE tasks SET tstatus=-100 WHERE id=%1").arg(QString::number(id_row));
            emit needExecQuery(query);
            QModelIndex srcIdx = select1.mapToSource(select1.index(y,9));
            model->addToCache(srcIdx.row(),srcIdx.column(),-100);
            ldr->deleteTask(id_task);
            tasklist.remove(id_row);
            model->updateRow(srcIdx.row());

            //запускаем новую задачу

            QUrl _url(select.data(select.index(i,1),100).toString());
            id_row = select.data(select.index(i,0),100).toInt();

            if(!plugproto.contains(_url.scheme().toLower())) //если протокол не поддерживается, то пропускаем эту задачу в очереди, поменяв её статус на ошибку
            {
                QString err = tr("Этот протокол не поддерживается. Проверьте наличие соответствующего плагина и его состояние.");
                int id = select.data(select.index(i,0),100).toInt();
                QString query = QString("UPDATE tasks SET status=%1, lasterror='%2' WHERE id=%3").arg(
                            QString::number((int)LInterface::ERROR_TASK),
                            err,
                            QString::number(id));

                emit needExecQuery(query);
                QModelIndex idx = select.mapToSource(select.index(i,9));
                model->addToCache(idx.row(),idx.column(),(int)LInterface::ERROR_TASK);
                idx = model->index(idx.row(),7);
                model->addToCache(idx.row(),idx.column(),err);
                model->updateRow(idx.row());
                continue;
            }

            startTaskNumber(id_row,_url,select.data(select.index(i,3),100).toString(),select.data(select.index(i,4),100).toLongLong());
        }
    }

    if(tasklist.size() > 0)startTrayIconAnimaion();
    else stopTrayIconAnimation();
    QTimer::singleShot(0,this,SLOT(updateStatusBar()));
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

    if(!selection->hasSelection()) // в случае, если ни одно определенное задание не выделено
    {
        status->hide();
        priority->hide();
        urllbl->hide();
        lasterror->hide();
        lefttime->hide();
        speed->setVisible(true);
        setEnabledTaskMenu(false);

        QSortFilterProxyModel filter;
        filter.setSourceModel(model);
        filter.setFilterKeyColumn(9);
        filter.setFilterRole(100);
        filter.setFilterRegExp("(^[1-4]$)");

        qint64 total_s, total_l, total_speed;
        total_s = total_l = total_speed = 0;

        QModelIndex cur_index;
        for(int i = 0; i < filter.rowCount(); i++)
        {
            int row = filter.mapToSource(filter.index(i,0)).row();
            cur_index = model->index(row,4);
            total_l += model->data(cur_index,100).toLongLong();
            cur_index = model->index(row,5);
            total_s += model->data(cur_index,100).toLongLong();
            cur_index = model->index(row,5);
        }
        QList<int>plug_keys = pluglist.keys();
        for(int y = 0; y < plug_keys.size(); y++) //обходим список плагинов, суммируем общие скорости скачивания
            total_speed += pluglist.value(plug_keys.value(y))->totalDownSpeed();

        QStringList spd_ = TItemModel::speedForHumans(total_speed);
        speed->setText(tr("Скорость: %1").arg(spd_.value(0)+spd_.value(1)));
        progress->setMaximum(100);
        int cur_val = total_s ? 100*total_l/total_s : 0;
        progress->setValue(cur_val);
        //int sec = total_speed ? (total_s-total_l)/total_speed : -1;
        //lefttime->setText(TItemModel::secForHumans(sec));
        //lefttime->setVisible(true);
    }
    else // если выделено определенное задание, то выводим по нему информацию
    {
        setEnabledTaskMenu(true);

        QModelIndex curIndex = selection->currentIndex();
        int row_id = sfmodel->mapToSource(curIndex).row();

        switch(model->index(row_id,9).data(100).toInt())
        {
        case LInterface::SEND_QUERY:
        case LInterface::ACCEPT_QUERY:
        case LInterface::REDIRECT:
        case LInterface::ON_LOAD: status->setPixmap(QPixmap(":/appimages/start_16x16.png")); speed->setVisible(true); break;
        case LInterface::STOPPING:
        case LInterface::ON_PAUSE: status->setPixmap(QPixmap(":/appimages/pause_16x16.png")); break;
        case LInterface::FINISHED: status->setPixmap(QPixmap(":/appimages/finish_16x16.png")); break;
        case -100: status->setPixmap(QPixmap(":/appimages/queue_16x16.png")); break;
        default:
            status->setPixmap(QPixmap(":/appimages/error_16x16.png"));
            break;
        }
        status->setVisible(true);

        ui->actionPVeryLow->setChecked(false);
        ui->actionPLow->setChecked(false);
        ui->actionPNormal->setChecked(false);
        ui->actionPHight->setChecked(false);
        ui->actionPVeryHight->setChecked(false);

        switch(model->index(row_id,13).data(100).toInt())
        {
        case 0: priority->setPixmap(QPixmap(":/appimages/pverylow_16x16.png")); ui->actionPVeryLow->setChecked(true); break;
        case 1: priority->setPixmap(QPixmap(":/appimages/plow_16x16.png")); ui->actionPLow->setChecked(true); break;
        case 2: priority->setPixmap(QPixmap(":/appimages/pnormal_16x16.png")); ui->actionPNormal->setChecked(true); break;
        case 3: priority->setPixmap(QPixmap(":/appimages/phight_16x16.png")); ui->actionPHight->setChecked(true); break;
        default:
            priority->setPixmap(QPixmap(":/appimages/pveryhight_16x16.png"));
            ui->actionPVeryHight->setChecked(true);
            break;
        }
        priority->setVisible(true);
        QUrl cur_url = QUrl::fromEncoded(model->index(row_id,1).data(Qt::DisplayRole).toByteArray());
        urllbl->setText(QString("<a href='%1'>%2</a>").arg(cur_url.toString(),TItemModel::shortUrl(cur_url.toString())));
        urllbl->setVisible(true);
        progress->setMaximum(100);
        int curVal = model->index(row_id,5).data(100).toLongLong() > 0 ? ((qint64)100*model->index(row_id,4).data(100).toLongLong()/model->index(row_id,5).data(100).toLongLong()) : 0;
        progress->setValue(curVal);
        lefttime->setText(model->index(row_id,15).data(Qt::DisplayRole).toString());
        lefttime->setVisible(true);
        lasterror->setText(model->index(row_id,7).data(100).toString());
        lasterror->setVisible(true);
        if(!model->index(row_id,14).data().toString().isEmpty()) speed->setText(tr("Скорость: %1").arg(model->index(row_id,14).data().toString()));
        else speed->hide();
    }

    QSortFilterProxyModel filter;
    filter.setSourceModel(model);
    filter.setFilterRole(100);
    filter.setFilterKeyColumn(9);
    filter.setFilterRegExp(QString("(^%1$)").arg(QString::number(LInterface::ON_LOAD)));
    onplay->setText(QString::number(filter.rowCount()));
    //if(filter.rowCount() > 0)startTrayIconAnimaion();
    //else stopTrayIconAnimation();
    filter.setFilterRegExp(QString("(^%1$)").arg(QString::number(LInterface::ON_PAUSE)));
    onpause->setText(QString::number(filter.rowCount()));
    filter.setFilterRegExp(QString("(^%1$)").arg(QString::number(-100)));
    onqueue->setText(QString::number(filter.rowCount()));
    filter.setFilterRegExp(QString("(^%1$)").arg(QString::number(LInterface::ERROR_TASK)));
    onerror->setText(QString::number(filter.rowCount()));
}

REXWindow::~REXWindow()
{
    delete ui;
    sched_flag = false;
    plugmgr->quit();
    plugmgr->wait(3000);

    lockProcess(false);
}

void REXWindow::importUrlFromFile(const QStringList &files)
{
    ImportDialog *dlg = new ImportDialog(downDir,this);
    QTimer::singleShot(0,dlg,SLOT(show()));
}

void REXWindow::showImportFileDialog()
{
    QFileDialog *dlg = new QFileDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setModal(false);
    dlg->setAcceptMode(QFileDialog::AcceptOpen);
    dlg->setFileMode(QFileDialog::ExistingFiles);
    dlg->setDirectory(QDir::home());
    dlg->setWindowTitle(tr("Файл для импорта"));
    dlg->setOption(QFileDialog::DontUseNativeDialog);

    connect(dlg,SIGNAL(filesSelected(QStringList)),this,SLOT(importUrlFromFile(QStringList)));

    dlg->show();
}

void REXWindow::calculateSpeed()
{
    int total = tasklist.size();
    QList<int> keys = pluglist.keys();
    LoaderInterface *ldr = 0;
    for(int i = 0; i < keys.size() && total > 0; ++i)
    {
        ldr = pluglist.value(keys.value(i));
        if(!ldr)continue;
        qint64 new_spd = down_speed*1024*(qint64)ldr->countTask()/(qint64)total;
        ldr->setDownSpeed(new_spd/8);
    }
}

bool REXWindow::event(QEvent *event)
{
    return QMainWindow::event(event);
}

void REXWindow::closeEvent(QCloseEvent *event)
{
    if(!isHidden() && sender() != this->findChild<QAction*>("exitAct"))
    {
        hide();
        settDlg->setWindowFlags(Qt::Window);
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

void REXWindow::acceptQAction(QAbstractButton *btn)
{
    QPointer<EMessageBox> dlg = qobject_cast<EMessageBox*>(sender());
    if(!dlg)return;

    QHash<QString, QString> params;
    QStringList _tmp_ = dlg->myParams().split("\r\n");
    if(!_tmp_.isEmpty())
        for(int i = 0; i < _tmp_.size(); ++i)
        {
            int index = _tmp_.value(i).indexOf(":");
            int cnt = _tmp_.value(i).size() - index-1;
            QString val;
            if(index >= 0) val = _tmp_.value(i).right(cnt);
            params.insert(_tmp_.value(i).split(":").value(0),val);
        }

    switch(dlg->myTypes())
    {
    case EMessageBox::AT_RENAME:
    {
        QFile file(params.value("curname"));
        if(dlg->buttonRole(qobject_cast<QPushButton*>(btn)) == EMessageBox::ApplyRole)
        {
            file.remove(params.value("newname"));
            file.rename(params.value("newname"));
        }
        else file.rename(params.value("rename"));

        QSqlQuery qr;
        qr.prepare("UPDATE tasks SET filename=:filename WHERE id=:id");
        qr.bindValue("filename",file.fileName());
        qr.bindValue("id",params.value("id"));
        if(!qr.exec())
        {
            ///запись в журнал ошибок
            qDebug()<<"void REXWindow::acceptQAction(1): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
        }
        updateTaskSheet();
        break;
    }
    case EMessageBox::AT_DOWNLOAD_ON_START:
    {
        if(dlg->buttonRole(qobject_cast<QPushButton*>(btn)) == EMessageBox::ApplyRole)
            startAllTasks();
        break;
    }
    case EMessageBox::AT_NONE:
    default: return;
    }
    dlg->deleteLater();
}

void REXWindow::scanTasksOnStart()
{
    QSqlQuery qr("SELECT COUNT(*) FROM tasks WHERE tstatus=0");
    if(!qr.exec())
    {
        ///запись в журнал ошибок
        qDebug()<<"void REXWindow::scanTasksOnStart(1): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
    }
    qr.next();
    if(qr.value(0).toInt() == 0)return;
    EMessageBox *question = new EMessageBox(this);
    question->setWindowTitle(tr("Продолжить закачку?"));
    question->setIcon(EMessageBox::Question);
    QPushButton *btn1 = question->addButton(tr("Продолжить все"),EMessageBox::ApplyRole);
    question->addButton(tr("Отмена"),EMessageBox::RejectRole);
    question->setDefaultButton(btn1);
    question->setText(tr("Есть незавершённые задания."));
    question->setInformativeText(tr("Для продолжения выполнения заданий нажмите \"Продолжить все\", для отмены - \"Отмена\""));
    question->setActionType(EMessageBox::AT_DOWNLOAD_ON_START);
    connect(question,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(acceptQAction(QAbstractButton*)));
    question->setModal(true);
    if(!isVisible()) question->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
    QTimer::singleShot(0,question, SLOT(show()));
}

void REXWindow::readSettings()
{
    max_tasks = settDlg->value("max_number_tasks").toInt();
    max_threads = settDlg->value("max_number_sections").toInt();
    downDir = settDlg->value("down_dir").toString();

    if(ui->actionVeryLow->isChecked()) down_speed = settDlg->value("s_vlow").toLongLong()*8;
    else if(ui->actionLow->isChecked()) down_speed = settDlg->value("s_low").toLongLong()*8;
    else if(ui->actionNormal->isChecked()) down_speed = settDlg->value("s_normal").toLongLong()*8;
    else down_speed = settDlg->value("s_hight").toLongLong()*8;

    QList<LoaderInterface*> plugins = pluglist.values();
    for(int i = 0; i < plugins.size(); ++i)
    {
        if(!plugins.value(i))continue;
        plugins.value(i)->setMaxErrorsOnTask(settDlg->value("max_number_errors").toInt());
        plugins.value(i)->setRetryCriticalError(settDlg->value("enable_ignore_errors").toBool());
        plugins.value(i)->setMaxSectionsOnTask(max_threads);
        plugins.value(i)->setUserAgent(settDlg->value("user_agent").toString());
    }

    setTaskCnt();
    calculateSpeed();
}

void REXWindow::selectSpeedRate(bool checked)
{
    QAction *act = qobject_cast<QAction*>(sender());
    if(!act)return;
    if(!checked)act->setChecked(true);

    if(act == ui->actionVeryLow)
    {
        ui->actionLow->setChecked(false);
        ui->actionNormal->setChecked(false);
        ui->actionHight->setChecked(false);
        down_speed = settDlg->value("s_vlow").toLongLong()*8;
        movie->setSpeed(25);
    }
    else if(act == ui->actionLow)
    {
        ui->actionVeryLow->setChecked(false);
        ui->actionNormal->setChecked(false);
        ui->actionHight->setChecked(false);
        down_speed = settDlg->value("s_low").toLongLong()*8;
        movie->setSpeed(50);
    }
    else if(act == ui->actionNormal)
    {
        ui->actionVeryLow->setChecked(false);
        ui->actionLow->setChecked(false);
        ui->actionHight->setChecked(false);
        down_speed = settDlg->value("s_normal").toLongLong()*8;
        movie->setSpeed(75);
    }
    else
    {
        ui->actionVeryLow->setChecked(false);
        ui->actionLow->setChecked(false);
        ui->actionNormal->setChecked(false);
        down_speed = settDlg->value("s_hight").toLongLong()*8;
        movie->setSpeed(100);
    }
    spdbtn->setIcon(act->icon());
    calculateSpeed();
}

void REXWindow::showTreeContextMenu(const QPoint &pos)
{
    QItemSelectionModel *selected = ui->treeView->selectionModel();
    if(!selected->selectedRows().size())
        return; //если ничего не выделено

    QModelIndex index = ui->treeView->indexAt(pos);
    index = treemodel->index(index.row(),1,index.parent());
    int id = treemodel->data(index,100).toInt();
    if(id < 1)
        return;

    if(id > 0 && id < 7)
        ui->actionDeleteCategory->setVisible(false);
    else ui->actionDeleteCategory->setVisible(true);

    if(id == 1)
        ui->actionCatProperties->setVisible(false);
    else ui->actionCatProperties->setVisible(true);

    QMenu *mnu = findChild<QMenu*>("treeMenu");
    if(mnu)mnu->popup(QCursor::pos());
}

void REXWindow::deleteCategory()
{
    QItemSelectionModel *selected = ui->treeView->selectionModel();
    if(!selected->selectedRows().size())
        return; //если ничего не выделено

    QModelIndex index = selected->selectedIndexes().value(0);
    QModelIndex parent_index = index.parent();
    index = treemodel->index(index.row(),1,index.parent());
    int id = treemodel->data(index,100).toInt();
    parent_index = treemodel->index(parent_index.row(),1,parent_index.parent());
    int parent_id = treemodel->data(parent_index,100).toInt();
    if(id < 7) return; //если выделены встроенные категории или фильтры

    QSqlQuery qr;
    qr.prepare("DELETE FROM categories WHERE id=:id"); //удаляем категорию
    qr.bindValue("id",id);
    if(!qr.exec())
    {
        ///запись в журнал ошибок
        qDebug()<<"void REXWindow::deleteCategory(1): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
    }

    qr.clear();
    qr.prepare("UPDATE categories SET parent_id=:pid WHERE parent_id=:id");
    qr.bindValue("pid",parent_id);
    qr.bindValue("id",id);
    if(!qr.exec())
    {
        ///запись в журнал ошибок
        qDebug()<<"void REXWindow::deleteCategory(2): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
    }

    qr.clear();
    qr.prepare("UPDATE tasks SET categoryid=:catid WHERE categoryid=:id"); //привязываем закачки удаляемой категории к её родителю
    qr.bindValue("catid",parent_id);
    qr.bindValue("id",id);
    if(!qr.exec())
    {
        ///запись в журнал ошибок
        qDebug()<<"void REXWindow::deleteCategory(3): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
    }
    model->silentUpdateModel();
    treemodel->removeRow(index.row(),index.parent());
}

void REXWindow::addCategory()
{
    int parent;

    QItemSelectionModel *selected = ui->treeView->selectionModel();
    if(!selected->hasSelection())
        parent = 1;
    else
    {
        QModelIndex index = selected->selectedRows().value(0);
        index = treemodel->index(index.row(),1,index.parent());
        parent = treemodel->data(index,100).toInt();
    }

    CategoryDialog *dlg = new CategoryDialog(this);
    dlg->setCategoryDir(downDir);
    dlg->setParentCategory(parent);
    connect(dlg,SIGNAL(canUpdateModel(QString,int,int,int)),this,SLOT(updateTreeModel(QString,int,int,int)));
    dlg->show();
}

void REXWindow::updateTreeModel(const QString cat_name, int row, int parent_id, int cat_id)
{
    QSqlQuery qr;

    QModelIndex parent = treemodel->indexById(parent_id);
    parent = treemodel->index(parent.row(),0,parent.parent());

    if(!cat_name.isEmpty())
    {
        qr.prepare("SELECT title, id, dir, extlist, parent_id FROM categories WHERE title=:title AND parent_id=:parent");
        qr.bindValue("title", cat_name);
        qr.bindValue("parent", parent_id);

        if(!qr.exec())
        {
            //запись в журнал ошибок
            qDebug()<<"void REXWindow::updateTreeModel(1): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
            treemodel->updateModel();
        }
        treemodel->insertRow(row,parent);
    }
    else
    {
        qr.prepare("SELECT title, id, dir, extlist, parent_id FROM categories WHERE id=:id");
        qr.bindValue("id", cat_id);

        if(!qr.exec())
        {
            //запись в журнал ошибок
            qDebug()<<"void REXWindow::updateTreeModel(2): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
            treemodel->updateModel();
        }
    }

    if(!qr.next())return;

    for(int i = 0; i < treemodel->columnCount(parent); i++)
        treemodel->setData(treemodel->index(row,i,parent),qr.value(i));
}

void REXWindow::categorySettings()
{
    int parent;

    QItemSelectionModel *selected = ui->treeView->selectionModel();
    if(!selected->hasSelection()) return;
    QModelIndex index = selected->selectedRows().value(0);
    index = treemodel->index(index.row(),1,index.parent());
    treemodel->index(index.row(),4,index.parent());
    parent = treemodel->data(treemodel->index(index.row(),4,index.parent()),100).toInt();
    QString catDir = treemodel->data(treemodel->index(index.row(),2,index.parent()),100).toString();
    QString ext = treemodel->data(treemodel->index(index.row(),3,index.parent()),100).toString();
    QString catTitle = treemodel->data(treemodel->index(index.row(),0,index.parent())).toString();
    if(catDir.isEmpty())
    {
        catDir = downDir;
        if(treemodel->data(index,100).toInt() > 0 && treemodel->data(index,100).toInt() < 7)
            catDir += "/" + catTitle;
    }

    CategoryDialog *dlg = new CategoryDialog(this);
    dlg->setParentCategory(parent);
    dlg->setCategory(treemodel->data(index,100).toInt(), parent);
    dlg->setCategoryDir(catDir);
    dlg->setCategoryExtList(ext);
    dlg->setCategoryTitle(catTitle);
    connect(dlg,SIGNAL(canUpdateModel(QString,int,int,int)),this,SLOT(updateTreeModel(QString,int,int,int)));
    dlg->show();
}

void REXWindow::setTaskCnt()
{
    QAction *sndr = qobject_cast<QAction*>(sender());
    if(!sndr)
        switch(settDlg->value("max_number_tasks").toInt())
        {
        case 1: sndr = ui->actionOneTask; break;
        case 2: sndr = ui->actionTwoTasks; break;
        case 3: sndr = ui->actionThreeTasks; break;
        case 4: sndr = ui->actionFourTasks; break;
        default: sndr = ui->actionFiveTasks; break;
        }

    if(sndr == ui->actionOneTask)
    {
        ui->actionTwoTasks->setChecked(false);
        ui->actionThreeTasks->setChecked(false);
        ui->actionFourTasks->setChecked(false);
        ui->actionFiveTasks->setChecked(false);
        settDlg->setSettingAttribute("max_number_tasks",1);
    }
    else if(sndr == ui->actionTwoTasks)
    {
        ui->actionOneTask->setChecked(false);
        ui->actionThreeTasks->setChecked(false);
        ui->actionFourTasks->setChecked(false);
        ui->actionFiveTasks->setChecked(false);
        settDlg->setSettingAttribute("max_number_tasks",2);
    }
    else if(sndr == ui->actionThreeTasks)
    {
        ui->actionTwoTasks->setChecked(false);
        ui->actionOneTask->setChecked(false);
        ui->actionFourTasks->setChecked(false);
        ui->actionFiveTasks->setChecked(false);
        settDlg->setSettingAttribute("max_number_tasks",3);
    }
    else if(sndr == ui->actionFourTasks)
    {
        ui->actionTwoTasks->setChecked(false);
        ui->actionThreeTasks->setChecked(false);
        ui->actionOneTask->setChecked(false);
        ui->actionFiveTasks->setChecked(false);
        settDlg->setSettingAttribute("max_number_tasks",4);
    }
    else if(sndr == ui->actionFiveTasks)
    {
        ui->actionTwoTasks->setChecked(false);
        ui->actionThreeTasks->setChecked(false);
        ui->actionFourTasks->setChecked(false);
        ui->actionOneTask->setChecked(false);
        settDlg->setSettingAttribute("max_number_tasks",5);
    }

    max_tasks = settDlg->value("max_number_tasks").toInt();
    sndr->setChecked(true);
    taskbtn->setIcon(sndr->icon());
    taskbtn->setText(sndr->text());
}

void REXWindow::showTaskDialog()
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())return; //если ничего не выделено, то выходим

    for(int i=0; i < select->selectedRows().length(); i++)
    {
        int id_row = select->selectedRows(0).value(i).data(100).toInt(); // id записи в базе данных
        if(dlglist.contains(id_row))
        {
            dlglist.value(id_row)->activateWindow();
            return;
        }

        TaskDialog *dlg = new TaskDialog(this);
        QModelIndex index = sfmodel->mapToSource(select->selectedRows(0).value(i));
        dlg->setSourceData(model, index, pluglist, tasklist);
        connect(dlg,SIGNAL(rejected()),this,SLOT(closeTaskDialog()));
        connect(dlg,SIGNAL(startTask(int)),this,SLOT(startTask(int)));
        connect(dlg,SIGNAL(stopTask(int)),this,SLOT(stopTask(int)));
        //connect(dlg,SIGNAL(redownloadTask(int)),this,SLOT(redownloadTask()));
        dlglist.insert(id_row, dlg);
        dlg->show();
    }
}

void REXWindow::closeTaskDialog()
{
    TaskDialog *dlg = qobject_cast<TaskDialog*>(sender());
    if(!dlg) return;

    int key = dlglist.key(dlg);
    dlglist.remove(key);
}

void REXWindow::prepareToQuit()
{
    stop_flag = true;
    stopAllTasks();
    saveSettings();
}

void REXWindow::setProxy(int id_task, int id_proto, bool global)
{
    if(settDlg->value("proxy_enable").toBool())
    {
        QUrl addr;
        addr.setHost(settDlg->value("proxy_address").toString());
        addr.setPort(settDlg->value("proxy_port").toInt());
        LInterface::ProxyType ptype = settDlg->value("enable_sockss").toBool() ? LInterface::PROXY_SOCKS5 : LInterface::PROXY_HTTP;
        QByteArray auth;
        QString authdata;
        if(settDlg->value("proxy_user").toString() != "")
        {
            auth.append(settDlg->value("proxy_user").toString() + ":" + settDlg->value("proxy_password").toString());
            authdata = auth.toBase64();
        }

        pluglist.value(id_proto)->setProxy(id_task, addr, ptype, authdata); //если настроен прокси, то указываем прокси для задачи
    }
}
