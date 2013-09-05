/*
Copyright (C) 2012-2013  Sarvaritdinov R.

This file is part of REXLoader.

REXLoader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

REXLoader is distributed in the hope that it will be useful,
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
    lock_mem = 0;

    QList<int> sz;
    QList<int> sz1;
    QDir libdir(QApplication::applicationDirPath());
    libdir.cdUp();
    apphomedir = QDir::homePath()+"/.config/rexloader";
    pluginDirs << libdir.absolutePath()+"/lib/rexloader/plugins" << libdir.absolutePath()+"/lib64/rexloader/plugins" << apphomedir+"/plugins";

    sz << 800 << 150;
    sz1 << 150 << 800;
    ui->splitter->setSizes(sz);
    ui->splitter_2->setSizes(sz1);

    trayicon = new QSystemTrayIcon(this);
    trayicon->setIcon(QIcon(":/appimages/trayicon.png"));
    trayicon->show();
    movie = new QMovie(this);
    movie->setFileName(":/appimages/onload.gif");
    movie->setCacheMode(QMovie::CacheAll);
    connect(movie,SIGNAL(updated(QRect)),this,SLOT(updateTrayIcon()));

    settDlg = new SettingsDialog(this);
    connect(settDlg,SIGNAL(newSettings()),this,SLOT(readSettings()));

    logmgr = new LogManager(this);
    lockProcess();
    openDataBase();

    plugmgr = new PluginManager(this);
    connect(plugmgr,SIGNAL(pluginStatus(bool)),this,SLOT(pluginStatus(bool)));
    connect(qApp,SIGNAL(aboutToQuit()),this,SLOT(prepareToQuit()));
    connect(this,SIGNAL(needExecQuery(QString)),plugmgr,SLOT(exeQuery(QString)));
    connect(plugmgr,SIGNAL(notifActionInvoked(QString)),this,SLOT(notifActAnalyzer(QString)));

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
        int quit_ok = QMessageBox::critical(this,windowTitle()+" - "+tr("Критическая ошибка"),tr("Не найден ни один плагин.\r\n Проверьте наличие файлов плагинов в директории '~/.config/rexloader/plugins' и '/usr/{local/}lib/rexloader/plugins'."));
        if(quit_ok == QMessageBox::Ok)QTimer::singleShot(0,this,SLOT(close()));
    }
    if(!plug_state.isEmpty())
        plugmgr->restorePluginsState(plug_state);
    plugmgr->loadLocale(QLocale::system());

    readSettings();

    plugmgr->notify("REXLoader",tr("Приложение успешно запущено"),5);
    if(ui->actionPoweroff->isChecked() || ui->actionHibernate->isChecked() || ui->actionSuspend->isChecked())
        plugmgr->notify(tr("Внимание!"),tr("Активирован режим <b>автоматического выключения ПК</b> по завершении всех заданий"),13);

    scanTasksOnStart();
}

void REXWindow::createInterface()
{
    //настраиваем таблицу
    model = new TItemModel(this);
    //model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->updateModel();
    sfmodel = new QSortFilterProxyModel(this);
    efmodel = new EFilterProxyModel(this);
    efmodel->setSourceModel(model);
    efmodel->addFilter(16,100, EFilterProxyModel::Equal,"");
    sfmodel->setSortRole(100);
    sfmodel->setSourceModel(efmodel);
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
    ui->tableView->hideColumn(14);
    ui->tableView->hideColumn(15);
    ui->tableView->hideColumn(16);

    ui->tableView->horizontalHeader()->moveSection(2,3);
    ui->tableView->horizontalHeader()->moveSection(17,11);
    ui->tableView->horizontalHeader()->moveSection(18,12);
    ui->tableView->scrollToBottom();

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

    //настраиваем модель отображения списка плагинов
    plugmodel = new PluginListModel(this);
    plugmodel->setSorces(&plugfiles,&pluglist,&plugproto);
    plugmgr->setPluginListModel(plugmodel);
    plugmgr->setTaskTable(ui->tableView);
    settDlg->setPlugListModel(plugmodel);
    settDlg->setPlugWidgets(plugmgr->getPlugWidgets());

    //настраиваем лог
    logmgr->setTabWidget(ui->tabWidget);
    connect(plugmgr,SIGNAL(messageAvailable(int,int,int,QString,QString)),logmgr,SLOT(appendLog(int,int,int,QString,QString)));
    connect(ui->tableView,SIGNAL(clicked(int)),logmgr,SLOT(manageTabs(int)));

    //добавляем строку поиска на панель меню
    search_line = new SearchLine(this);
    search_line->resize(150,ui->menuBar->height()-2);
    search_line->move(size().width()-136, ui->menuBar->pos().y() + 1);
    search_line->setSourceSortFilterModel(sfmodel);

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
    connect(ui->actionPoweroff,SIGNAL(triggered()),this,SLOT(setPostActionMode()));
    connect(ui->actionHibernate,SIGNAL(triggered()),this,SLOT(setPostActionMode()));
    connect(ui->actionSuspend,SIGNAL(triggered()),this,SLOT(setPostActionMode()));
    connect(qApp->clipboard(),SIGNAL(dataChanged()),this,SLOT(scanClipboard()));
    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(showAbout()));
    connect(ui->actionpluginsShow,SIGNAL(triggered()),this,SLOT(showSettDialog()));

    //кнопка-меню для выбора скорости
    spdbtn = new QToolButton(this);
    ui->actionHight->setChecked(true);
    spdbtn->setMenu(ui->menu_6);
    spdbtn->setPopupMode(QToolButton::InstantPopup);
    spdbtn->setMinimumWidth(45);
    spdbtn->setToolTip(tr("Регулятор скорости"));
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
    taskbtn->setMinimumSize(45,32);
    taskbtn->setToolTip(tr("Количество одновременно закачиваемых заданий"));
    ui->mainToolBar->addWidget(taskbtn);

    //создаем и настраиваем плавающее окошко
    fwnd = new FloatingWindow();
    connect(this,SIGNAL(taskStarted(int)),fwnd,SLOT(startTask(int)));
    connect(this,SIGNAL(taskStopped(int)),fwnd,SLOT(stopTask(int)));
    connect(this,SIGNAL(taskData(int,qint64,qint64,QString)),fwnd,SLOT(taskData(int,qint64,qint64,QString)));
    connect(this,SIGNAL(TotalDowSpeed(qint64)),fwnd,SLOT(currentSpeed(qint64)));
    connect(fwnd,SIGNAL(selectedTask(int)),this,SLOT(showTaskDialogById(int)));

    if(settDlg->value("show_float_window").toBool())fwnd->show();

    //настроиваем значок в трее
    QMenu *traymenu = new QMenu(this);
    traymenu->setObjectName("traymenu");
    QAction *trayact = new QAction(this);
    trayact->setObjectName("showHideAct");
    trayact->setText(tr("Скрыть"));
    connect(trayact,SIGNAL(triggered()),this,SLOT(showHideSlot()));
    traymenu->addAction(ui->actionAdd_URL);
    traymenu->addAction(ui->actionImportURL);
    traymenu->addSeparator();
    traymenu->addAction(ui->actionStartAll);
    traymenu->addAction(ui->actionStopAll);
    traymenu->addSeparator();
    traymenu->addMenu(ui->menu_6);
    traymenu->addMenu(taskmenu);
    traymenu->addSeparator();
    traymenu->addAction(ui->actionAppSettings);
    QMenu *submnu = traymenu->addMenu(tr("Плавающее окно"));
    submnu->addAction(fwnd->findChild<QAction*>("ShowAlways"));
    submnu->addAction(fwnd->findChild<QAction*>("ShowDownloadOnly"));
    traymenu->addSeparator();
    traymenu->addAction(trayact);
    trayact = new QAction(this);
    trayact->setObjectName("exitAct");
    trayact->setText(tr("Выход"));
    trayact->setIcon(QIcon(":/appimages/exit.png"));
    connect(trayact,SIGNAL(triggered()),this,SLOT(close()));
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
    tblMenu->addMenu(plugmgr->filePluginMenu());
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

    //настраиваем меню колонок таблицы заданий
    QMenu *tblHdrMenu = new QMenu(this);
    tblHdrMenu->setObjectName("tblHdrMenu");
    QAction *titleact = new QAction(tr("Видимые колонки"),tblHdrMenu);
    titleact->setEnabled(false);
    tblHdrMenu->addAction(titleact);
    tblHdrMenu->addSeparator();
    for(int i = 0; i < model->columnCount(QModelIndex()); ++i)
    {
        if(i == 3 || i == 9) continue;

        if(!ui->tableView->isColumnHidden(i) && !model->headerData(i,Qt::Horizontal,Qt::DisplayRole).isNull())
        {
            QAction *act = new QAction(model->headerData(i,Qt::Horizontal,Qt::DisplayRole).toString(),tblHdrMenu);
            act->setObjectName(QString::number(i));
            act->setCheckable(true);
            tblHdrMenu->addAction(act);
            act->setChecked(true);
            connect(act,SIGNAL(triggered()),this,SLOT(showHideTableColumn()));
        }
    }
    ui->tableView->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView->horizontalHeader(),SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showTableHeaderContextMenu(QPoint)));
}

void REXWindow::showTableContextMenu(const QPoint &pos)
{
    Q_UNUSED(pos)
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

void REXWindow::showTableHeaderContextMenu(const QPoint &pos)
{
    Q_UNUSED(pos)
    QMenu *mnu = findChild<QMenu*>("tblHdrMenu");
    if(!mnu)
        return;

    for(int i = 0; i < model->columnCount(QModelIndex()); ++i)
    {
        QAction *act = mnu->findChild<QAction*>(QString::number(i));
        if(act) act->setChecked(!ui->tableView->isColumnHidden(i));
    }

    mnu->popup(QCursor::pos());
}

void REXWindow::setTaskFilter(const QModelIndex &index)
{
    search_line->clearSearch();
    QModelIndex mindex = treemodel->index(index.row(),1,index.parent());
    int id_cat = treemodel->data(mindex,100).toInt();
    if(id_cat == 1 || id_cat == -1)
    {
        sfmodel->setFilterRegExp("");

        if(!efmodel->containsFilter(16,100,EFilterProxyModel::Equal,""))
        {
            efmodel->prepareToRemoveFilter(16);
            efmodel->prepareFilter(16,100,EFilterProxyModel::Equal,"");
            efmodel->execPrepared();
            ui->tableView->hideColumn(16);
        }
        return;
    }

    int key_column = 10;
    int model_column = 1;
    int subcat_cnt = treemodel->rowCount(index);
    QString filter = "";

    if(id_cat < 0 && id_cat > -8) //если выделен элемент фильра статусов
    {
        key_column = 9;
        model_column = 3;
        mindex = treemodel->index(index.row(),3,index.parent());
        filter = QString("(^%1$)").arg(QString::number(treemodel->data(mindex,100).toInt()));

        if(!efmodel->containsFilter(16,100,EFilterProxyModel::Equal,""))
        {
            efmodel->prepareToRemoveFilter(16);
            efmodel->prepareFilter(16,100,EFilterProxyModel::Equal,"");
            efmodel->execPrepared();
            ui->tableView->hideColumn(16);
        }
    }
    else if(id_cat <= -8 && id_cat >= -12) //если выделен элемент фильтра удалённых заданий
    {
        sfmodel->setFilterRegExp(""); //сбрасываем фильтр для статусов и категорий

        efmodel->prepareToRemoveFilter(16);
        efmodel->prepareFilter(16,100, EFilterProxyModel::Not | EFilterProxyModel::Equal,"");
        efmodel->execPrepared();
        ui->tableView->showColumn(16);
    }
    else //если выделен элемент фильтра по категориям
    {
        filter = QString("(^%1$)").arg(QString::number(id_cat));

        if(!efmodel->containsFilter(16,100,EFilterProxyModel::Equal,""))
        {
            efmodel->prepareToRemoveFilter(16);
            efmodel->prepareFilter(16,100,EFilterProxyModel::Equal,"");
            efmodel->execPrepared();
            ui->tableView->hideColumn(16);
        }
    }
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

void REXWindow::openTask(const QString &path)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
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

void REXWindow::openTaskDir(const QString &path)
{
    QFileInfo flinfo(path);
    QString pth = path;
    if(!flinfo.isDir())pth = flinfo.absolutePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(pth));
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

    QModelIndex curIdx = sfmodel->index(ui->tableView->currentIndex().row(),9);
    curIdx = efmodel->mapToSource(sfmodel->mapToSource(curIdx));
    if(ui->tableView->selectionModel()->selectedRows().size() < 2 && model->data(curIdx,100).toInt() == LInterface::FINISHED)
        ui->menu_7->setEnabled(false);
    else
        ui->menu_7->setEnabled(true);
    if(ui->tableView->selectionModel()->selectedRows().size() < 2 && model->data(curIdx,100).toInt() != LInterface::FINISHED)
        ui->actionRedownload->setEnabled(false);
    else
        ui->actionRedownload->setEnabled(true);
    ui->actionOpenDir->setEnabled(true);
    ui->actionOpenTask->setEnabled(true);
    ui->actionTaskPropert->setEnabled(true);
}

QStringList REXWindow::getActionMap(int type, const QString &opt) const
{
    QStringList acts;
    if(type & AB_OPENDIR)
    {
        acts << QString("#OpenDir:") + opt;
        acts << tr("Открыть папку");
    }
    if(type & AB_OPENFILE)
    {
        acts << QString("#OpenFile:") + opt;
        acts << tr("Открыть файл");
    }
    if(type & AB_RETRY)
    {
        acts << QString("#Retry:") + opt;
        acts << tr("Повторить попытку");
    }

    return acts;
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
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
        qDebug()<<"void REXWindow::setTaskPriority(1) " + qr.executedQuery() + " Error:" + qr.lastError().text();
    }
    updateTaskSheet();
}

void REXWindow::setTaskPriority(int id, int prior)
{
    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("UPDATE tasks SET priority=:priority WHERE id=:id");
    qr.bindValue("priority",prior);
    qr.bindValue("id",id);
    if(!qr.exec())
    {
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
        qDebug()<<"void REXWindow::setTaskPriority(1)" + qr.executedQuery() + " Error:" + qr.lastError().text();
    }
    updateTaskSheet();
}

void REXWindow::saveSettings()
{
    QSettings settings(apphomedir+"/rexloader.ini", QSettings::IniFormat,this);

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

    settings.beginGroup("Floating Window");
    settings.setValue("Position",fwnd->pos());
    settings.setValue("Geometry",fwnd->saveGeometry());
    settings.setValue("ShowAlways",fwnd->showWindowsMode());
    settings.setValue("GraphMode",fwnd->renderGraphMode());
    settings.endGroup();

    settings.beginGroup("Search");
    settings.setValue("State", search_line->saveSearchState());
    settings.endGroup();

    settings.sync();

#ifdef Q_OS_LINUX
    //реализация автозапуска приложения по стандарту freedesktop.org
    if(settDlg->value("autostart").toBool() && !QFile::exists(QDir::homePath()+"/.config/autostart/rexloader.desktop"))
    {
        QFile fl(QDir::homePath()+"/.config/autostart/rexloader.desktop");
        if(fl.open(QFile::WriteOnly))
        {
            QString desktop = QString("[Desktop Entry]\r\n"
                                      "Name=REXLoader\r\n"
                                      "Name[x-test]=xxREXLoaderxx\r\n"
                                      "GenericName=Download Manager\r\n"
                                      "Exec=rexloader\r\n"
                                      "Icon=rexloader\r\n"
                                      "Categories=Network;Qt;FileTransfer;\r\n"
                                      "Type=Application\r\n");
            fl.write(desktop.toAscii());
            fl.close();
        }
        else qDebug()<<"void REXWindow::saveSettings(1) Error: can't create desktop file";
    }
    else if(!settDlg->value("autostart").toBool() && QFile::exists(QDir::homePath()+"/.config/autostart/rexloader.desktop"))
        QFile::remove(QDir::homePath()+"/.config/autostart/rexloader.desktop");
#endif
}

void REXWindow::loadSettings()
{
    QFileInfo info(apphomedir+"/rexloader.ini");
    if(!info.exists())
    {
        plugmgr->isFirstRun();
        return;
    }

    QSettings settings(apphomedir+"/rexloader.ini", QSettings::IniFormat,this);
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

    settings.beginGroup("Floating Window");
    fwnd->move(settings.value("Position",QPoint()).toPoint());
    fwnd->restoreGeometry(settings.value("Geometry",QByteArray()).toByteArray());
    fwnd->setRenderGraphMode(settings.value("GraphMode",1).toInt());
    fwnd->setShowWindowMode(settings.value("ShowAlways",true).toBool());
    settings.endGroup();

    settings.beginGroup("Search");
    search_line->restoreSearchState(settings.value("State",QByteArray()).toByteArray());
    settings.endGroup();

    ui->tableView->hideColumn(0);
    ui->tableView->hideColumn(1);
    ui->tableView->hideColumn(4);
    ui->tableView->hideColumn(7);
    ui->tableView->hideColumn(8);
    ui->tableView->hideColumn(10);
    ui->tableView->hideColumn(11);
    ui->tableView->hideColumn(13);
    ui->tableView->hideColumn(14);
    ui->tableView->hideColumn(15);
    ui->tableView->hideColumn(16);

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
        dlg->activateWindow();
        dlg->setFocus();
    }
}

void REXWindow::openDataBase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName(apphomedir+"/tasks.db");
    if(!db.open())
    {
        setEnabled(false);
        int quit_ok = QMessageBox::critical(this,windowTitle()+" - "+tr("Критическая ошибка"),tr("Невозможно открыть файл базы данных.\r\n Это критическая ошибка, приложение будет закрыто.\r\n Проверьте свои права доступа к директории '~/.config/rexloader'."));
        if(quit_ok == QMessageBox::Ok)QTimer::singleShot(0,this,SLOT(close()));
    }

    QSqlQuery qr;
    qr.prepare("UPDATE tasks SET tstatus=:newstatus WHERE tstatus=:tstatus;");
    qr.bindValue("newstatus",-100);
    qr.bindValue("tstatus",LInterface::ON_LOAD);

    if(!qr.exec())
    {
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
        qDebug()<<"void REXWindow::openDataBase(1)" + qr.executedQuery() + " Error:" + qr.lastError().text();
    }

    dbconnect = db.connectionName();
}

void REXWindow::scanNewTaskQueue()
{
    QSqlQuery qr;
    if(!qr.exec("SELECT * FROM newtasks;"))
    {
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
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
                    dlg->activateWindow();
                }
                default: break;
                }
            }
            else if(!settDlg->value("noadd_unsupported").toBool() || plugproto.contains(url.scheme().toLower()))
            {
                dlg = new AddTaskDialog(downDir, this);
                connect(dlg,SIGNAL(addedNewTask()),this,SLOT(updateTaskSheet()));
                dlg->setValidProtocols(plugproto);
                dlg->setNewUrl(qr.value(1).toString());
                dlg->setParams(qr.value(3).toString());
                dlg->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
                if(!settDlg->value("notshow_adding").toBool())
                {
                    dlg->show();
                    dlg->activateWindow();
                }
                else
                {
                    dlg->addTask();
                    if(!plugproto.contains(url.scheme().toLower()))
                    {
                        QString notif = tr("Протокол <b>%1</b> не поддерживается. Невозможно скачать файл по URL <b>%2</b>").arg(url.scheme().toLower(),url.toString());
                        plugmgr->notify(tr("Ошибка"), notif, 10, QStringList(), NotifInterface::WARNING);
                        notif = notif.replace(QRegExp("(<b>)|(</b>)"),"");
                        plugmgr->appendLog(-1,0, LInterface::MT_WARNING,notif,QString());
                    }
                }
            }
            else
            {
                QString notif = tr("Протокол <b>%1</b> не поддерживается. Невозможно скачать файл по URL <b>%2</b>").arg(url.scheme().toLower(),url.toString());
                plugmgr->notify(tr("Ошибка"), notif, 10, QStringList(), NotifInterface::WARNING);
                notif = notif.replace(QRegExp("(<b>)|(</b>)"),"");
                plugmgr->appendLog(-1,0, LInterface::MT_WARNING,notif,QString());
            }
        }

        QSqlQuery qr1;
        qr1.prepare("DELETE FROM newtasks WHERE id=:id");
        qr1.bindValue("id",qr.value(0));

        if(!qr1.exec())
        {
            logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                              tr("Ошибка выполнения SQL запроса"),
                              tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                              );
            qDebug()<<"Error: void REXWindow::scanNewTaskQueue(): "<<qr.lastError().text();
            return;
        }
    }
}

void REXWindow::loadPlugins()
{
    QString dbfile = apphomedir+"/tasks.db";
    plugmgr->setDatabaseFile(dbfile);

    plugmgr->setDefaultSettings(max_tasks, max_threads, down_speed, settDlg->value("attempt_interval").toInt());
    plugmgr->setPlugDir(pluginDirs);
    plugmgr->setPlugLists(&plugfiles, &pluglist, &plugproto, &tasklist);
    plugmgr->start();
}

void REXWindow::lockProcess(bool flag)
{
    if(flag)
    {
        if(!lock_mem)
        {
            lock_mem = new QSharedMemory("rexloader",this);
            lock_mem->create(64);
        }
        else
        {
            lock_mem->attach();
            QByteArray data(( const char*)lock_mem->data(),lock_mem->size());
            QString sdata = data;
            sdata = sdata.split("\r\n").value(1,0);

            if(sdata == "1")
            {
                if(isVisible())
                    activateWindow();
                else QTimer::singleShot(0,this,SLOT(showHideSlot()));
            }
        }

        lock_mem->lock();
        QString dtime = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss") + "\r\n0";
        memcpy(lock_mem->data(),dtime.toAscii().data(),dtime.toAscii().size());
        lock_mem->unlock();
    }
    else
        lock_mem->deleteLater();
}

void REXWindow::scheduler()
{
    if(!sched_flag)return;
    lockProcess(true);

    scanNewTaskQueue();
    syncTaskData();
    manageTaskQueue();
    if(!trayicon->isSystemTrayAvailable() && trayicon->isVisible()) //исправление исчезновения значка в лотке после падения plasma-desktop на KD
        trayicon->hide();

    if(trayicon->isSystemTrayAvailable() && !trayicon->isVisible())
        trayicon->show();


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
    dlg->activateWindow();
}

void REXWindow::showHideSlot(QSystemTrayIcon::ActivationReason type)
{
    if(type == QSystemTrayIcon::DoubleClick && settDlg->value("tray_doubleclick").toBool()) showHideSlot();
    else if(type == QSystemTrayIcon::Trigger && !settDlg->value("tray_doubleclick").toBool()) showHideSlot();
}

void REXWindow::showHideSlot()
{
    QAction *act = findChild<QAction*>("showHideAct");
    if(isVisible())
    {
        bool vflag = false;
        if(settDlg->isVisible()) vflag = true;
        setHidden(true);
        if(vflag)settDlg->show();
        preStat = windowState();
        act->setText(tr("Восстановить"));
    }
    else
    {
        setHidden(false);
        setWindowState(preStat);
        activateWindow();
        act->setText(tr("Скрыть"));
    }
}

void REXWindow::startTaskNumber(int id_row, const QUrl &url, const QString &filename, qint64 totalload)
{
    if(!plugproto.contains(url.scheme().toLower())) return;

    QFileInfo flinfo(filename);
    int id_proto = plugproto.value(url.scheme().toLower());
    int id_task = 0;

    QSortFilterProxyModel select;
    select.setSourceModel(model);
    select.setFilterKeyColumn(0);
    select.setFilterRole(100);
    select.setFilterFixedString(QString::number(id_row));
    QModelIndex srcIdx = select.mapToSource(select.index(0,0));

    if(totalload > 0) //если файл на докачке, а не новый
    {
        if(QFile::exists(filename) && flinfo.isFile() && filename.right(5) == ".rldr") //если локальный файл существует
        {
            id_task = pluglist.value(id_proto)->loadTaskFile(filename); // id задачи
            if(id_task) //если плагин удачно прочитал метаданные и добавил задачу
            {
                tasklist.insert(id_row, id_task + id_proto*100);
                calculateSpeed();

                setProxy(id_task, id_proto);

                pluglist.value(id_proto)->setAdvancedOptions(id_task,model->data(model->index(srcIdx.row(),14),100).toString());
                plugmgr->startDownload(id_task + id_proto*100);
                emit taskStarted(id_task + id_proto*100);
                if(settDlg->value("show_taskdialog").toBool()) showTaskDialog(id_row);
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

    QString qr = QString("UPDATE tasks SET downtime='', lasterror='', speed_avg='' WHERE id=%1").arg(QString::number(id_row));
    emit needExecQuery(qr);

    model->addToCache(srcIdx.row(),6,QString());
    model->addToCache(srcIdx.row(),7,QString());
    model->addToCache(srcIdx.row(),11,QString());
    model->addToCache(srcIdx.row(),9,LInterface::ON_LOAD);
    model->updateRow(srcIdx.row());

    QString fldir = (flinfo.isDir() || flinfo.fileName() == "noname.html") ? flinfo.absolutePath():flinfo.absoluteFilePath();
    flinfo.setFile(fldir);
    if(!flinfo.isDir() && fldir.right(5) != ".rldr") fldir += "." + QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + ".rldr";
    tasklist.insert(id_row, id_task + id_proto*100);
    pluglist.value(id_proto)->setTaskFilePath(id_task,fldir);
    pluglist.value(id_proto)->setAdvancedOptions(id_task,model->data(model->index(srcIdx.row(),14),100).toString());
    calculateSpeed();
    //pluglist.value(id_proto)->startDownload(id_task);
    setProxy(id_task, id_proto);
    plugmgr->startDownload(id_task + id_proto*100);
    emit taskStarted(id_task + id_proto*100);
    if(settDlg->value("show_taskdialog").toBool()) showTaskDialog(id_row);
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
        dlg.setInformativeText(tr("Чтобы подтвердить удаление нескольких заданий нажмите <b>\"Ok\"</b> или <b>\"Отмена\"</b> для отмены удаления."));
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

    QString cdtime = QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss");
    qr.prepare("UPDATE tasks SET arch=:dtime WHERE"+where);
    qr.bindValue(":dtime",cdtime);

    if(!qr.exec())
    {
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
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

        QModelIndex index = sfmodel->mapToSource(select->selectedRows(9).value(i));
        index = efmodel->mapToSource(index);
        model->addToCache(index.row(),9,-100);
    }
    if(where.isEmpty())return;

    emit needExecQuery("UPDATE tasks SET tstatus=-100, lasterror='' WHERE " + where);

    for(int i=0; i < select->selectedRows().length(); i++)
    {
        QModelIndex index = sfmodel->mapToSource(select->selectedRows(9).value(i));
        index = efmodel->mapToSource(index);
        model->updateRow(index.row());
    }
    manageTaskQueue();
}

void REXWindow::startTask(int id)
{
    QSqlQuery qr(QSqlDatabase::database());
    qr.prepare("UPDATE tasks SET tstatus=-100, lasterror='' WHERE id=:id AND tstatus <> :tstatus");
    qr.bindValue("id",id);
    qr.bindValue("tstatus",(int)LInterface::FINISHED);

    if(!qr.exec())
    {
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
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
}

void REXWindow::startAllTasks()
{    
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
}

void REXWindow::redownloadTask()
{
    QItemSelectionModel *select = ui->tableView->selectionModel();
    if(!select->hasSelection())return; //если ничего не выделено, то выходим
    QString where;

    for(int i=0; i < select->selectedRows().length(); i++)
    {
        if(select->selectedRows(9).value(i).data(100).toInt() != LInterface::FINISHED)continue;
        if(where.isEmpty())
            where = QString("id=%1").arg(QString::number(select->selectedRows(0).value(i).data(100).toInt()));
        else where += QString(" OR id=%1").arg(QString::number(select->selectedRows(0).value(i).data(100).toInt()));

        QModelIndex index = sfmodel->mapToSource(select->selectedRows(9).value(i));
        index = efmodel->mapToSource(index);
        model->addToCache(index.row(),9,-100);
        model->updateRow(index.row());
    }
    QString query = QString("UPDATE tasks SET tstatus=-100, currentsize=NULL, totalsize=NULL, speed_avg=NULL, downtime=NULL, datecreate='%1' WHERE %2").arg(
                QDateTime::currentDateTime().toString("yyyy-MM-ddThh:mm:ss"),
                where);

    emit needExecQuery(query);
    manageTaskQueue();
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
            logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                              tr("Ошибка выполнения SQL запроса"),
                              tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                              );
            qDebug()<<"void REXWindow::stopTask(1): SQL: " + qr.executedQuery() + "; Error: " + qr.lastError().text();
        }
    }

    updateTaskSheet();
    for(int i=0; i < select->selectedRows().length(); i++)
    {
        QModelIndex index = sfmodel->mapToSource(select->selectedRows(9).value(i));
        index = efmodel->mapToSource(index);
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
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
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
            logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                              tr("Ошибка выполнения SQL запроса"),
                              tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                              );
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

        if(upd_block.contains(id_row))
            continue;

        int gid_task = tasklist.value(id_row);
        int id_task = gid_task%100;
        int id_proto = gid_task/100;

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
                logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                                  tr("Ошибка выполнения SQL запроса"),
                                  tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                                  );
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
        qint64 speedAvg = downtime ? (model->data(speedAvgId,100).toLongLong()*(qint64)downtime + speed/1024)/(qint64)(downtime+1) : 0;
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
                        filepath.replace("'","''"),
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
            logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                              tr("Ошибка при скачивании файла '%1': %2 (Код ошибки: %3)").arg(model->data(model->index(index.row(),3),Qt::DisplayRole).toString(),errStr,QString::number(errno_)),
                              QString());
            plugmgr->notify(tr("Ошибка"),
                            tr("Ошибка при скачивании файла <b>%1</b>: <b>%2</b>").arg(model->data(model->index(index.row(),3),Qt::DisplayRole).toString(),errStr,QString::number(errno_)));

            ldr->deleteTask(id_task);
            if(dlglist.contains(id_row)) dlglist.value(id_row)->close();
            emit taskStopped(gid_task);
            tasklist.remove(id_row);
            calculateSpeed();
        }
        else
        {
            if(tstatus == LInterface::FINISHED)
            {
                QString newFilename = filepath.right(5) == ".rldr" ? filepath.left(filepath.size()-20) : filepath;
                QFile fl(filepath);
                int id_proto = gid_task/100;

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
                    QString params = QString("curname:%1\r\nnewname:%2\r\nrename:%3\r\nid:%4\r\nmime:%5").arg(
                                filepath,
                                newFilename,
                                reFilename,
                                QString::number(id_row),
                                pluglist.value(id_proto)->mimeType(id_task)
                                );
                    question->setParams(params);
                    question->setModal(false);
                    if(!isVisible())question->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
                    question->show();
                    question->activateWindow();
                }
                else
                {
                    fl.rename(newFilename);
                    filepath = newFilename;
                    QFileInfo flinfo(filepath);
                    logmgr->appendLog(-1,0,LInterface::MT_INFO,tr("Скачивание файла %1 завершено").arg(flinfo.fileName()),QString());
                    logmgr->deleteTaskLogLater(id_row);

                    plugmgr->notify(tr("Задание завершено"),tr("Скачивание файла <b>%1</b> завершено").arg(flinfo.fileName()),10,
                                    getActionMap(AB_OPENDIR | AB_OPENFILE, filepath));

                    checkFileType(pluglist.value(id_proto)->mimeType(id_task),filepath);
                }

                if(dlglist.contains(id_row)) dlglist.value(id_row)->close();
            }
            QString query = QString("UPDATE tasks SET totalsize='%1', currentsize='%2', filename='%3', downtime=%4, tstatus=%5, speed_avg='%6' WHERE id=%7").arg(
                        QString::number(totalsize),
                        QString::number(totalload),
                        filepath.replace("'","''"),
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

            emit taskData(gid_task, totalsize, totalload, model->data(index, Qt::ToolTipRole).toString());
        }

        if(tstatus == LInterface::ON_PAUSE || tstatus == LInterface::FINISHED)
        {
            ldr->deleteTask(id_task);
            emit taskStopped(gid_task);
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
    {
        trayicon->showMessage(tr("REXLoader"),tr("Все задания завершены."));

        if(ui->actionPoweroff->isChecked() || ui->actionHibernate->isChecked() || ui->actionSuspend->isChecked())
        {
            EMessageBox *question = new EMessageBox(this);
            question->setWindowTitle(tr("Завершить работу ПК?"));
            question->setIcon(EMessageBox::Question);
            QPushButton *btn1 = question->addButton(tr("Выключить ПК"),EMessageBox::ApplyRole);
            question->addButton(tr("Отмена"),EMessageBox::RejectRole);
            question->setDefaultButton(btn1);
            question->setText(tr("Выключить ПК после завершения всех заданий?"));
            question->setInformativeText(tr("Для завершения работы ПК нажмите \"Выключить ПК\", для отмены - \"Отмена\""));
            question->setActionType(EMessageBox::AT_SHUTDOWN);
            connect(question,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(acceptQAction(QAbstractButton*)));
            question->setModal(false);
            question->setDefaultTimeout(50);
            if(!isVisible()) question->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
            question->show();
        }
    }
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
        QString flname = select.data(select.index(i,3),Qt::DisplayRole).toString();

        QUrl _url = QUrl::fromEncoded(select.data(select.index(i,1),100).toByteArray());
        if(/*!plugproto.contains(_url.scheme().toLower()) || */!plugproto.value(_url.scheme().toLower(),0))
        {
            QString err = tr("Протокол '%1' не поддерживается. Проверьте наличие соответствующего плагина и его состояние.")
                    .arg(_url.scheme().toUpper());
            QString query = QString("UPDATE tasks SET tstatus=%1, lasterror='%2' WHERE id=%3").arg(
                        QString::number((int)LInterface::ERROR_TASK),
                        err.replace("'","''"),
                        QString::number(id_row));
            emit needExecQuery(query);

            logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                              tr("Ошибка при загрузке файла %1").arg(flname),
                              err);
            plugmgr->notify(tr("Ошибка"),tr("Ошибка при загрузке файла <b>%1</b>").arg(flname));

            QModelIndex srcIdx = select.mapToSource(select.index(i,9));
            model->addToCache(srcIdx.row(),srcIdx.column(),(int)LInterface::ERROR_TASK);
            srcIdx = model->index(srcIdx.row(),7);
            model->addToCache(srcIdx.row(),srcIdx.column(),err);
            model->updateRow(srcIdx.row());
            continue;
        }

        logmgr->appendLog(-1,0,
                          LInterface::MT_INFO,
                          tr("Загрузка файла %1 начинается").arg(flname),
                          QString());
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
            QString flname = select1.data(select1.index(y,3),Qt::DisplayRole).toString();
            int id_task = tasklist.value(id_row)%100;
            int id_proto = tasklist.value(id_row)/100;

            LoaderInterface *ldr = pluglist.value(id_proto);

            //останавливаем найденную задачу, удаляем её из менеджера закачек
            ldr->stopDownload(id_task);
            logmgr->appendLog(-1,0,
                              LInterface::MT_INFO,
                              tr("Загрузка файла %1 приостановлена").arg(flname),
                              QString());
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
            flname = select.data(select.index(i,3),Qt::DisplayRole).toString();

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

            logmgr->appendLog(-1,0,
                              LInterface::MT_INFO,
                              tr("Загрузка файла %1 начинается").arg(flname),
                              QString());
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

    qint64 total_speed = 0;
    QList<int>plug_keys = pluglist.keys();
    for(int y = 0; y < plug_keys.size(); y++) //обходим список плагинов, суммируем общие скорости скачивания
        total_speed += pluglist.value(plug_keys.value(y))->totalDownSpeed();
    emit TotalDowSpeed(total_speed);

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
        int row_id = efmodel->mapToSource(sfmodel->mapToSource(curIndex)).row();

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

        switch(sfmodel->sourceModel()->index(row_id,13).data(100).toInt())
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
        QUrl cur_url = QUrl::fromEncoded(sfmodel->sourceModel()->index(row_id,1).data(Qt::DisplayRole).toByteArray());
        urllbl->setText(QString("<a href='%1'>%2</a>").arg(cur_url.toString(),TItemModel::shortUrl(cur_url.toString())));
        urllbl->setVisible(true);
        progress->setMaximum(100);
        int curVal = sfmodel->sourceModel()->index(row_id,5).data(100).toLongLong() > 0 ? ((qint64)100*sfmodel->sourceModel()->index(row_id,4).data(100).toLongLong()/sfmodel->sourceModel()->index(row_id,5).data(100).toLongLong()) : 0;
        progress->setValue(curVal);
        lefttime->setText(tr("Осталось: %1").arg(sfmodel->sourceModel()->index(row_id,18).data(Qt::DisplayRole).toString()));
        lefttime->setVisible(true);
        lasterror->setText(sfmodel->sourceModel()->index(row_id,7).data(100).toString());
        lasterror->setVisible(true);
        if(!sfmodel->sourceModel()->index(row_id,17).data().toString().isEmpty()) speed->setText(tr("Скорость: %1").arg(sfmodel->sourceModel()->index(row_id,17).data().toString()));
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

    fwnd->close();
    delete fwnd;

    lockProcess(false);
}

void REXWindow::importUrlFromFile(const QStringList &files)
{
    ImportDialog *dlg = new ImportDialog(files,this);
    connect(dlg,SIGNAL(addedNewTask()),this,SLOT(updateTaskSheet()));
    dlg->setDownDir(settDlg->value("down_dir").toString());
    QStringList protocols = plugproto.keys();
    QString proto;
    foreach(proto,protocols)
        dlg->addProtocol(proto);

    dlg->import();
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

void REXWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if(!ui->menuBar->actionAt(QPoint(size().width()-search_line->width()-5,5)))
        search_line->move(size().width()-search_line->width()-5, ui->menuBar->pos().y() + 1);
    else
    {
        int xpos = size().width()-search_line->width()-5;
        while(ui->menuBar->actionAt(QPoint(xpos,5)))
              xpos += 5;
        search_line->move(xpos, ui->menuBar->pos().y() + 1);
    }
    search_line->resize(search_line->width(),ui->menuBar->height()-2);
}

void REXWindow::closeEvent(QCloseEvent *event)
{
    if(!isHidden() && sender() != this->findChild<QAction*>("exitAct"))
    {
        hide();
        QAction *act = findChild<QAction*>("showHideAct");
        act->setText(tr("Восстановить"));
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
        QFileInfo flinfo;
        if(dlg->buttonRole(qobject_cast<QPushButton*>(btn)) == EMessageBox::ApplyRole)
        {
            flinfo.setFile(params.value("newname"));
            logmgr->appendLog(-1,0,LInterface::MT_INFO,tr("Замена файла %1").arg(flinfo.fileName()),QString());
            file.remove(params.value("newname"));
            file.rename(params.value("newname"));
            logmgr->appendLog(-1,0,LInterface::MT_INFO,tr("Скачивание файла %1 завершено").arg(flinfo.fileName()),QString());
            plugmgr->notify(tr("Задание завершено"),tr("Скачивание файла <b>%1</b> завершено").arg(flinfo.fileName()),10,
                            getActionMap(AB_OPENDIR | AB_OPENFILE, params.value("newname")));
        }
        else
        {
            QFileInfo oldfl(params.value("rename"));
            file.rename(params.value("rename"));
            flinfo.setFile(params.value("newname"));
            logmgr->appendLog(-1,0,LInterface::MT_INFO,tr("Файл %1 сохранен как %2").arg(flinfo.fileName(),oldfl.fileName()),QString());
            logmgr->appendLog(-1,0,LInterface::MT_INFO,tr("Скачивание файла %1 завершено").arg(oldfl.fileName()),QString());
            plugmgr->notify(tr("Задание завершено"),tr("Файл <b>%1</b> скачан, переименован и сохранен как <b>%2</b>").arg(flinfo.fileName(),oldfl.fileName()),10,
                            getActionMap(AB_OPENDIR | AB_OPENFILE, params.value("rename")));
        }

        QSqlQuery qr;
        qr.prepare("UPDATE tasks SET filename=:filename WHERE id=:id");
        qr.bindValue("filename",file.fileName());
        qr.bindValue("id",params.value("id"));
        if(!qr.exec())
        {
            logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                              tr("Ошибка выполнения SQL запроса"),
                              tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                              );
            qDebug()<<"void REXWindow::acceptQAction(1): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
        }
        logmgr->deleteTaskLogLater(params.value("id").toInt());
        updateTaskSheet();
        checkFileType(params.value("mime"),file.fileName());
        break;
    }
    case EMessageBox::AT_DOWNLOAD_ON_START:
    {
        if(dlg->buttonRole(qobject_cast<QPushButton*>(btn)) == EMessageBox::ApplyRole)
            startAllTasks();
        break;
    }
    case EMessageBox::AT_SHUTDOWN:
    {
        if(dlg->buttonRole(qobject_cast<QPushButton*>(btn)) == EMessageBox::ApplyRole)
            QTimer::singleShot(0,this,SLOT(shutDownPC()));
        break;
    }
    case EMessageBox::AT_URL_IMPORT:
    {
        if(dlg->buttonRole(qobject_cast<QPushButton*>(btn)) == EMessageBox::ApplyRole)
            importUrlFromFile(QStringList(params.value("file")));
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
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
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
    if(!QDir().exists(downDir))
    {
        if(downDir.isEmpty())
            downDir = QDir::homePath()+tr("/Загрузки");
        QDir().mkpath(downDir);
    }

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
        plugins.value(i)->setAttemptInterval(settDlg->value("attempt_interval").toInt());
    }

    model->setSpdFormat(settDlg->value("speed_on_kBps").toBool());
    fwnd->setSpeedFormat(settDlg->value("speed_on_kBps").toBool());
    model->setRowColor((int)LInterface::ON_PAUSE, settDlg->value("on_pause_color").value<QColor>());
    model->setRowColor((int)LInterface::ON_LOAD, settDlg->value("on_load_color").value<QColor>());
    model->setRowColor((int)LInterface::ERROR_TASK, settDlg->value("on_error_color").value<QColor>());
    model->setRowColor(-100, settDlg->value("on_queue_color").value<QColor>());
    model->setRowColor(LInterface::FINISHED, settDlg->value("on_finish_color").value<QColor>());

    model->setRowFont((int)LInterface::ON_PAUSE, settDlg->value("on_pause_font").value<QFont>());
    model->setRowFont((int)LInterface::ON_LOAD, settDlg->value("on_load_font").value<QFont>());
    model->setRowFont((int)LInterface::ERROR_TASK, settDlg->value("on_error_font").value<QFont>());
    model->setRowFont(-100, settDlg->value("on_queue_color").value<QFont>());
    model->setRowFont(LInterface::FINISHED, settDlg->value("on_finish_font").value<QFont>());

    model->setRowFontColor((int)LInterface::ON_PAUSE, settDlg->value("on_pause_font_color").value<QColor>());
    model->setRowFontColor((int)LInterface::ON_LOAD, settDlg->value("on_load_font_color").value<QColor>());
    model->setRowFontColor((int)LInterface::ERROR_TASK, settDlg->value("on_error_font_color").value<QColor>());
    model->setRowFontColor(-100, settDlg->value("on_queue_font_color").value<QColor>());
    model->setRowFontColor(LInterface::FINISHED, settDlg->value("on_finish_font_color").value<QColor>());

    ui->tableView->setWordWrap(settDlg->value("table_word_wrap").toBool());
    ui->tableView->resizeRowsToContents();
    ui->tableView->scroll(0,1); // прокручиваем на 1 пиксел вниз для того, чтобы заставить вьюшку перерисовать строки
    ui->tableView->scroll(0,-1); // возвращаем скроллер на место

    logmgr->setDeleteInterval(settDlg->value("log_life_time").toInt());
    logmgr->setLogColor((int)LInterface::MT_INFO,settDlg->value("log_info_color").value<QColor>());
    logmgr->setLogFont((int)LInterface::MT_INFO,settDlg->value("log_info_font").value<QFont>());
    logmgr->setLogFontColor((int)LInterface::MT_INFO,settDlg->value("log_info_font_color").value<QColor>());

    logmgr->setLogColor((int)LInterface::MT_WARNING,settDlg->value("log_warning_color").value<QColor>());
    logmgr->setLogFont((int)LInterface::MT_WARNING,settDlg->value("log_warning_font").value<QFont>());
    logmgr->setLogFontColor((int)LInterface::MT_WARNING,settDlg->value("log_warning_font_color").value<QColor>());

    logmgr->setLogColor((int)LInterface::MT_ERROR,settDlg->value("log_error_color").value<QColor>());
    logmgr->setLogFont((int)LInterface::MT_ERROR,settDlg->value("log_error_font").value<QFont>());
    logmgr->setLogFontColor((int)LInterface::MT_ERROR,settDlg->value("log_error_font_color").value<QColor>());

    logmgr->setLogColor((int)LInterface::MT_OUT,settDlg->value("log_out_color").value<QColor>());
    logmgr->setLogFont((int)LInterface::MT_OUT,settDlg->value("log_out_font").value<QFont>());
    logmgr->setLogFontColor((int)LInterface::MT_OUT,settDlg->value("log_out_font_color").value<QColor>());

    logmgr->setLogColor((int)LInterface::MT_IN,settDlg->value("log_in_color").value<QColor>());
    logmgr->setLogFont((int)LInterface::MT_IN,settDlg->value("log_in_font").value<QFont>());
    logmgr->setLogFontColor((int)LInterface::MT_IN,settDlg->value("log_in_font_color").value<QColor>());

    logmgr->setLogAutoSave(settDlg->value("log_autosave").toBool(),settDlg->value("log_dir").toString());
    logmgr->setMaxStringCount(settDlg->value("log_max_strings").toInt());
    LogTreeModel::setColorsFontStylesEnabled(!settDlg->value("log_use_system_style").toBool());

    ui->actionPoweroff->setChecked(settDlg->value("poweroff").toBool());
    ui->actionHibernate->setChecked(settDlg->value("hibernate").toBool());
    ui->actionSuspend->setChecked(settDlg->value("suspend").toBool());

    if(settDlg->value("show_float_window").toBool())
    {
        fwnd->disableWindow(false);
        fwnd->show();
        fwnd->moveToAllDesktops(true);
    }
    else
    {
        fwnd->disableWindow(true);
        fwnd->hide();
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
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
        qDebug()<<"void REXWindow::deleteCategory(1): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
    }

    qr.clear();
    qr.prepare("UPDATE categories SET parent_id=:pid WHERE parent_id=:id");
    qr.bindValue("pid",parent_id);
    qr.bindValue("id",id);
    if(!qr.exec())
    {
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
        qDebug()<<"void REXWindow::deleteCategory(2): SQL:" + qr.executedQuery() + " Error: " + qr.lastError().text();
    }

    qr.clear();
    qr.prepare("UPDATE tasks SET categoryid=:catid WHERE categoryid=:id"); //привязываем закачки удаляемой категории к её родителю
    qr.bindValue("catid",parent_id);
    qr.bindValue("id",id);
    if(!qr.exec())
    {
        logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                          tr("Ошибка выполнения SQL запроса"),
                          tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                          );
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
            logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                              tr("Ошибка выполнения SQL запроса"),
                              tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                              );
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
            logmgr->appendLog(-1,0,LInterface::MT_ERROR,
                              tr("Ошибка выполнения SQL запроса"),
                              tr("Запрос: %1\nОшибка: %2").arg(qr.executedQuery(),qr.lastError().text())
                              );
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
    QString catTitle = treemodel->data(treemodel->index(index.row(),0,index.parent()),101).toString();
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
    //taskbtn->setText(sndr->text());
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

        TaskDialog *dlg = new TaskDialog(downDir,this);
        QModelIndex index = sfmodel->mapToSource(select->selectedRows(0).value(i));
        index = efmodel->mapToSource(index);
        dlg->setSourceData(model, index, pluglist, tasklist);
        connect(dlg,SIGNAL(rejected()),this,SLOT(closeTaskDialog()));
        connect(dlg,SIGNAL(startTask(int)),this,SLOT(startTask(int)));
        connect(dlg,SIGNAL(stopTask(int)),this,SLOT(stopTask(int)));
        connect(dlg,SIGNAL(setPriority(int,int)),this,SLOT(setTaskPriority(int,int)));
        dlglist.insert(id_row, dlg);
        dlg->show();
    }
}

void REXWindow::showTaskDialog(int id_row)
{
    if(dlglist.contains(id_row))
    {
        dlglist.value(id_row)->activateWindow();
        return;
    }

    QSortFilterProxyModel filter;
    filter.setSourceModel(model);
    filter.setFilterKeyColumn(0);
    filter.setFilterRole(100);
    filter.setFilterFixedString(QString::number(id_row));
    QModelIndex index = filter.mapToSource(filter.index(0,0));

    TaskDialog *dlg = new TaskDialog(downDir,this);
    dlg->setSourceData(model, index, pluglist, tasklist);
    connect(dlg,SIGNAL(rejected()),this,SLOT(closeTaskDialog()));
    connect(dlg,SIGNAL(startTask(int)),this,SLOT(startTask(int)));
    connect(dlg,SIGNAL(stopTask(int)),this,SLOT(stopTask(int)));
    connect(dlg,SIGNAL(setPriority(int,int)),this,SLOT(setTaskPriority(int,int)));
    dlglist.insert(id_row, dlg);
    dlg->show();
}

void REXWindow::showTaskDialogById(int id_task)
{
    QList<int> tskid_list = tasklist.values();
    if(tskid_list.contains(id_task))
        showTaskDialog(tasklist.key(id_task));
}

void REXWindow::closeTaskDialog()
{
    TaskDialog *dlg = qobject_cast<TaskDialog*>(sender());
    if(!dlg) return;

    int key = dlglist.key(dlg);
    dlglist.remove(key);
}

void REXWindow::setPostActionMode()
{
    QAction *act = qobject_cast<QAction*>(sender());

    if(act)
    {
        if(act == ui->actionPoweroff)
        {
            if(act->isChecked())
            {
                ui->actionHibernate->setChecked(false);
                ui->actionSuspend->setChecked(false);
                settDlg->setSettingAttribute("suspend",ui->actionSuspend->isChecked());
                settDlg->setSettingAttribute("hibernate",ui->actionHibernate->isChecked());
            }
            else
            {
                settDlg->setSettingAttribute("poweroff",false);
                return;
            }
        }
        else if(act == ui->actionSuspend)
        {
            if(act->isChecked())
            {
                ui->actionHibernate->setChecked(false);
                ui->actionPoweroff->setChecked(false);
                settDlg->setSettingAttribute("poweroff",ui->actionPoweroff->isChecked());
                settDlg->setSettingAttribute("hibernate",ui->actionHibernate->isChecked());
            }
            else
            {
                settDlg->setSettingAttribute("suspend",false);
                return;
            }
        }
        else if(act == ui->actionHibernate)
        {
            if(act->isChecked())
            {
                ui->actionPoweroff->setChecked(false);
                ui->actionSuspend->setChecked(false);
                settDlg->setSettingAttribute("poweroff",ui->actionPoweroff->isChecked());
                settDlg->setSettingAttribute("suspend",ui->actionSuspend->isChecked());
            }
            else
            {
                settDlg->setSettingAttribute("hibernate",false);
                return;
            }
        }

        QMessageBox *dlg = new QMessageBox(this);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setIcon(QMessageBox::Question);
        dlg->setWindowTitle(tr("Повторять действие завершения работы ПК?"));
        dlg->setText(tr("Вы установили опцию автоматического завершения работы ПК по завершению всех заданий. "
                        "Хотите чтобы программа всегда выполняла данное действие автоматизации?"));
        dlg->addButton(tr("Да, всегда выключать ПК"),QMessageBox::AcceptRole);
        QPushButton *defBtn = dlg->addButton(tr("Нет, выключить единоразово"),QMessageBox::RejectRole);
        dlg->setDefaultButton(defBtn);
        connect(dlg,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(setPostActionMode()));
        dlg->show();
        return;
    }

    QMessageBox *dlg = qobject_cast<QMessageBox*>(sender());

    if(dlg)
    {
        if(dlg->buttonRole(dlg->clickedButton()) == QMessageBox::AcceptRole)
        {
            settDlg->setSettingAttribute("poweroff",ui->actionPoweroff->isChecked());
            settDlg->setSettingAttribute("suspend",ui->actionSuspend->isChecked());
            settDlg->setSettingAttribute("hibernate",ui->actionHibernate->isChecked());
        }
    }
}

void REXWindow::shutDownPC()
{
    bool appQuit = false;
    if(ui->actionPoweroff->isChecked())
        appQuit = ShutdownManager::shutdownPC();
    else if(ui->actionHibernate->isChecked())
        ShutdownManager::hibernatePC();
    else if(ui->actionSuspend->isChecked())
        ShutdownManager::suspendPC();
    else
        return;

    if(appQuit)
        qApp->quit();
}

void REXWindow::showAbout()
{
    QString text = QString("<h2>REXLoader<br><small>%1<br>by Sarvaritdinov Ravil (aka RA9OAJ)<small></h2>").arg(APP_VERSION) + tr(
                "Это приложение - свободное программное обеспечение и распространяется по лицензии GNU/GPL-3. "
                "Разработка идет при участии <a href='http://kubuntu.ru/'>Русского сообщества Kubuntu</a>, "
                "сайт разработчика - <a href='http://spolab.ru/'>Лаборатория Свободного программного обеспечения (Лаборатория СПО)</a>."
                "<hr>Уважаемые пользователи! Я приглашаю принять участие всех заинтересовавшихся данной программой в её дальнейшей разработке. "
                "Для этого вам достаточно связаться со мной по электронной почте <a href='mailto:ra9oaj@gmail.com'>ra9oaj@gmail.com</a>, "
                "либо зарегистрироваться на сайте <a href='http://spolab.ru/'>Лаборатория СПО</a> и связаться с помощью личного сообщения.<br>"
                "Выражаю особую благодарность активному пользователю Русского сообщества Kubuntu - <b>Дмитрию Перлову (aka DarkneSS)</b>, ставшему первым Maintainer'ом "
                "этого ПО в различных дистрибутивах GNU/Linux."
                );
    QMessageBox::about(this,tr("О программе"),text);
}

void REXWindow::notifActAnalyzer(const QString &act)
{
    if(!act.indexOf("#OpenDir:"))
        openTaskDir(act.split("#OpenDir:").value(1));
    else if(!act.indexOf("#OpenFile"))
        openTask(act.split("#OpenFile:").value(1));
}

void REXWindow::checkFileType(const QString &mime, const QString &filepath)
{
    if(mime.indexOf(QRegExp("(text)|(html)")) != -1)
    {
        EMessageBox *question = new EMessageBox(this);
        question->setIcon(EMessageBox::Question);
        question->setDefaultTimeout(0);
        QPushButton *btn1 = question->addButton(tr("Импортировать"),EMessageBox::ApplyRole);
        question->addButton(tr("Отмена"),EMessageBox::RejectRole);
        question->setDefaultButton(btn1);
        QFileInfo flinfo(filepath);
        question->setText(tr("Файл <b>%1</b> является текстовым/html, вы можете импортировать URL из файла").arg(flinfo.fileName()));
        question->setInformativeText(tr("Для импорта нажмите \"Импортировать\", для отмены - \"Отмена\"."));
        question->setActionType(EMessageBox::AT_URL_IMPORT);
        question->setParams(QString("file:%1").arg(filepath));
        connect(question,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(acceptQAction(QAbstractButton*)));
        question->setModal(false);
        if(!isVisible())question->setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
        question->show();
        question->activateWindow();
    }
}

void REXWindow::showSettDialog()
{
    QAction *act = qobject_cast<QAction*>(sender());

    if(act == ui->actionpluginsShow)
        settDlg->selectCurrentSubsettings(SettingsDialog::PLUGINS);

    settDlg->show();
}

void REXWindow::showHideTableColumn()
{
    QAction *act = qobject_cast<QAction*>(sender());
    if(!act) return;

    act->isChecked() ? ui->tableView->showColumn(act->objectName().toInt()) : ui->tableView->hideColumn(act->objectName().toInt());
}

void REXWindow::startUpdateTaskProc(int id)
{
    int task_id = tasklist.value(id,0);
    QPair<QString,QString> old_data;

    QSortFilterProxyModel smodel;
    smodel.setSourceModel(model);
    smodel.setFilterRole(100);
    smodel.setFilterKeyColumn(0);
    smodel.setFilterFixedString(QString::number(id));
    if(!sfmodel->rowCount())
        return;

    QModelIndex curidx = smodel.mapToSource(smodel.index(0,1));
    old_data.first = curidx.data(100).toString();
    curidx = curidx.model()->index(curidx.row(),3);
    old_data.second = curidx.data(100).toString();
    upd_block.insert(id,old_data);

    int id_proto = task_id/100;
    if(pluglist.contains(id_proto))
        plugmgr->stopDownload(task_id);
}

void REXWindow::endUpdateTaskProc(int id)
{
    QSortFilterProxyModel smodel;
    smodel.setSourceModel(model);
    smodel.setFilterRole(100);
    smodel.setFilterKeyColumn(0);
    smodel.setFilterFixedString(QString::number(id));
    if(!sfmodel->rowCount())
        return;

    updateTaskSheet();

    if(upd_block.contains(id))
    {
        updated_tasks.append(id);
        QTimer::singleShot(3000,this,SLOT(startUpdatedTask()));
        return;
    }
}

void REXWindow::startUpdatedTask()
{
    if(updated_tasks.isEmpty())
        return;

    int id = updated_tasks.first();

    QSortFilterProxyModel smodel;
    smodel.setSourceModel(model);
    smodel.setFilterRole(100);
    smodel.setFilterKeyColumn(0);
    smodel.setFilterFixedString(QString::number(id));
    if(!sfmodel->rowCount())
        return;

    updateTaskSheet();

    int task_id = tasklist.value(id,0);
    int id_proto = task_id/100;

    QModelIndex curidx = smodel.mapToSource(smodel.index(0,1));
    QUrl url = QUrl::fromEncoded(curidx.data(100).toByteArray());
    while(upd_block.value(id).first != curidx.data(100).toString()) //переписываем метаданные файла
    {
        QFile file(upd_block.value(id).second);
        if(!file.open(QIODevice::ReadWrite)) break;

        if(file.size() > 8)
        {
            QDataStream fl(&file);
            qint64 pos = file.size() - 8;
            file.seek(pos);
            fl >> pos;
            int length = 0;

            file.seek(pos);
            QString header = file.readLine(3);
            if(header != "\r\n")
            {
                file.close();
                break;
            }

            header = file.readLine(254);
            if(header.indexOf("RExLoader") != 0){file.close(); break;}

            QString fversion = header.split(" ").value(1);
            if(fversion != "0.1a.1\r\n"){file.close(); break;}

            pos = file.pos();
            fl >> length;
            QByteArray inbuffer;
            inbuffer.resize(length);
            fl.readRawData(inbuffer.data(),length); //считываем URL
            inbuffer.clear();
            inbuffer.resize(file.size() - file.pos());
            fl.readRawData(inbuffer.data(), file.size() - file.pos()); // копируем в буфер все остальные неизменные данные
            file.seek(pos);
            length = url.toEncoded().size();
            fl << (qint32)length;
            fl.writeRawData(url.toEncoded().data(),length); //записываем в файл новый URL
            fl.writeRawData(inbuffer,inbuffer.size()); //записываем остальные скопированные данные
            pos = file.pos();
            file.close();
            file.resize(pos);
        }
       break;
    }

    curidx = smodel.mapToSource(smodel.index(0,3));
    QString filename = curidx.data(100).toString();
    if(upd_block.value(id).second != filename && QFile::exists(upd_block.value(id).second)) // если изменилось название/расположение, то переименовывем/переносим файл
    {
        QFileInfo flinfo(filename);
        if(!QFile::exists(flinfo.absolutePath()))
        {
            QDir curdir;
            curdir.mkpath(flinfo.absolutePath());
        }
        QFile::rename(upd_block.value(id).second,filename);
    }


    if(pluglist.contains(id_proto))
    {
        int new_id = 0;
        pluglist.value(id_proto)->deleteTask(task_id%100);
        id_proto = plugproto.contains(url.scheme().toLower()) ? plugproto.value(url.scheme().toLower()) : 0;

        if(id_proto)
        {
            if(QFile::exists(filename))
                new_id = pluglist.value(id_proto)->loadTaskFile(filename);
            if(!new_id)
                new_id = pluglist.value(id_proto)->addTask(url);

            tasklist.insert(id,id_proto*100+new_id);
            plugmgr->startDownload(id_proto*100+new_id);
        }
    }

    upd_block.remove(id);
    updated_tasks.removeFirst();
    updateTaskSheet();
}

void REXWindow::prepareToQuit()
{
    stop_flag = true;
    stopAllTasks();
    saveSettings();
}

void REXWindow::setProxy(int id_task, int id_proto, bool global)
{
    Q_UNUSED(global)
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
