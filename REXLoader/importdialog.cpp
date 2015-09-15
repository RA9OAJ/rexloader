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

#include "importdialog.h"
#include "ui_importdialog.h"

ImportDialog::ImportDialog(const QStringList &files, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

    mydb = QSqlDatabase::database();

    connect(ui->selectAllButton,SIGNAL(released()),this,SLOT(selectAll()));
    connect(ui->deselectAllButton,SIGNAL(released()),this,SLOT(deselectAll()));
    connect(ui->invertSelectionButton,SIGNAL(released()),this,SLOT(invertSelection()));
    connect(ui->categoryBox,SIGNAL(currentIndexChanged(int)),this,SLOT(locationSelected(int)));
    connect(ui->tableWidget,SIGNAL(itemSelectionChanged()),this,SLOT(selectionChanged()));
    connect(ui->categoryBox,SIGNAL(currentIndexChanged(int)),this,SLOT(setCategory(int)));
    connect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
    connect(ui->dirButton,SIGNAL(released()),this,SLOT(showDirDialog()));
    connect(ui->pushButton,SIGNAL(released()),this,SLOT(addTasks()));
    connect(ui->pushButton_2,SIGNAL(released()),this,SLOT(addTasks()));
    connect(ui->pushButton_3,SIGNAL(released()),this,SLOT(reject()));

    imp_files = files;
    fndurl = 0;
    initialize();
}

ImportDialog::ImportDialog(const QStringList &files, QSqlDatabase &db_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

    mydb = db_;

    imp_files = files;
    initialize();
}

ImportDialog::~ImportDialog()
{
    delete ui;
}

void ImportDialog::setDownDir(const QString &dir)
{
    downDir = dir;
    ui->locationEdit->setText(dir);
    loadDatabaseData();
}

void ImportDialog::addProtocol(const QString &proto)
{
    if(protocols.contains(proto))
        return;
    protocols.append(proto);
}

void ImportDialog::loadDatabaseData()
{
    QSqlQuery qr(mydb);
    qr.exec("SELECT * FROM categories");
    int otherId = 0;

    while(qr.next())
    {
        QString cattitle;
        if(qr.value(1).toString() == "#downloads")continue;
        else if(qr.value(1).toString() == "#archives")
        {
            cattitle = tr("Архивы");
            dirs.insert(qr.value(0).toInt(),(qr.value(2).toString() != "" ? qr.value(2).toString() : downDir)+"/"+cattitle);
        }
        else if(qr.value(1).toString() == "#apps")
        {
            cattitle = tr("Приложения");
            dirs.insert(qr.value(0).toInt(),(qr.value(2).toString() != "" ? qr.value(2).toString() : downDir)+"/"+cattitle);
        }
        else if(qr.value(1).toString() == "#audio")
        {
            cattitle = tr("Аудио");
            dirs.insert(qr.value(0).toInt(),(qr.value(2).toString() != "" ? qr.value(2).toString() : downDir)+"/"+cattitle);
        }
        else if(qr.value(1).toString() == "#video")
        {
            cattitle = tr("Видео");
            dirs.insert(qr.value(0).toInt(),(qr.value(2).toString() != "" ? qr.value(2).toString() : downDir)+"/"+cattitle);
        }
        else if(qr.value(1).toString() == "#other")
        {
            cattitle = tr("Другое");
            otherId = qr.value(0).toInt();
            dirs.insert(qr.value(0).toInt(),(qr.value(2).toString() != "" ? qr.value(2).toString() : downDir));
        }
        else {cattitle = qr.value(1).toString(); dirs.insert(qr.value(0).toInt(),qr.value(2).toString());}

        ui->categoryBox->addItem(cattitle, qr.value(0).toInt());
    }
    ui->categoryBox->setCurrentIndex(ui->categoryBox->findData(QVariant(otherId)));
}

int ImportDialog::getCategory(const QString &file)
{
    QString fl = file;
    QUrl nurl = QUrl::fromEncoded(fl.toLatin1());
    QFileInfo flinfo(nurl.toString(QUrl::RemoveFragment | QUrl::RemoveQuery));

    QSqlQuery qr(mydb), qr1(mydb);
    qr.prepare("SELECT id,extlist FROM categories WHERE extlist LIKE :ext1 OR extlist LIKE :ext2 OR extlist LIKE :ext3 OR extlist LIKE :ext4");
    qr1.prepare("SELECT id,extlist FROM categories WHERE extlist LIKE '%\%U%' ESCAPE '\\' ");
    QString ext_ = flinfo.suffix().toLower();
    qr.bindValue("ext1", QString("% %1 %").arg(ext_));
    qr.bindValue("ext2", QString("%1 %").arg(flinfo.suffix()));
    qr.bindValue("ext3", QString("% %1").arg(flinfo.suffix()));
    qr.bindValue("ext4", QString("%1").arg(flinfo.suffix()));
    if(!qr.exec())
    {
        //тут запись в журнал ошибок
        qDebug()<<"void AddTaskDialog::getCategory(1): Error: "<<qr.lastError().text();
        close();
        return -1;
    }
    if(!qr1.exec())
    {
        //тут запись в журнал ошибок
        qDebug()<<"void AddTaskDialog::getCategory(2): Error: "<<qr1.lastError().text();
        close();
        return -1;
    }

    int catId = 6;
    int chars_max_cnt = 0;
    while(qr1.next())
    {
        QStringList extlst = qr1.value(1).toString().split(" "); //получаем список расширений и хостов
        for(int i = 0; extlst.indexOf(QRegExp("%U[:./\\w+]+"),i) >= 0;)
        {
            int index = extlst.indexOf(QRegExp("%U[:./\\w+]+"),i);
            int chars_cnt = extlst.value(index).size();
            if(chars_cnt <= chars_max_cnt)continue;
            QString curhost = extlst.value(index).right(chars_cnt-2);
            if(fl.indexOf(curhost,0,Qt::CaseInsensitive) >= 0)
            {
                chars_max_cnt = chars_cnt;
                catId = qr1.value(0).toInt();
            }
            i = index + 1;
        }
    }
    if(catId == 6 && qr.next())
        catId = qr.value(0).toInt();

    return catId;
}

bool ImportDialog::contains(const QString &url) const
{
    int rows = ui->tableWidget->rowCount();
    for(int i = 0; i < rows; ++i)
        if(QTableWidgetItem *itm = ui->tableWidget->item(i,0))
            if(itm->text() == url)
                return true;

    return false;
}

void ImportDialog::selectAll()
{
    QTableWidgetSelectionRange rng(0,0,ui->tableWidget->rowCount()-1,0);
    disconnect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
    ui->tableWidget->setRangeSelected(rng,true);
    connect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
}

void ImportDialog::deselectAll()
{
    QTableWidgetSelectionRange rng(0,0,ui->tableWidget->rowCount()-1,0);
    disconnect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
    ui->tableWidget->setRangeSelected(rng,false);
    connect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
}

void ImportDialog::invertSelection()
{
    QList<QTableWidgetSelectionRange> rngs = ui->tableWidget->selectedRanges();
    disconnect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
    selectAll();
    QTableWidgetSelectionRange rng;
    foreach(rng,rngs)
        ui->tableWidget->setRangeSelected(rng,false);
    connect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
}

void ImportDialog::locationSelected(int id)
{
    ui->locationEdit->setText(dirs.value(ui->categoryBox->itemData(id).toInt()));
}

void ImportDialog::selectionChanged()
{
    QList<QTableWidgetSelectionRange> rngs = ui->tableWidget->selectedRanges();
    if(rngs.isEmpty())
        return;

    int current = rngs.last().bottomRow();
    QTableWidgetItem *itm = ui->tableWidget->item(current,0);
    ui->categoryBox->setCurrentIndex(ui->categoryBox->findData(itm->data(100)));

    if(!(itm->data(101).toString().isEmpty()))
        ui->locationEdit->setText(itm->data(101).toString());
    else locationSelected(ui->categoryBox->currentIndex());

    disconnect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
    ui->downloadLater->setTristate(false);
    ui->downloadLater->setCheckState(Qt::Unchecked);
    if(itm->data(102).toBool() == false)
        ui->downloadLater->setCheckState(Qt::Unchecked);
    else
        ui->downloadLater->setCheckState(Qt::Checked);


    bool checkstat = itm->data(102).toBool();

    QTableWidgetSelectionRange rng;
    foreach(rng,rngs)
        for(int i = rng.topRow(); i <= rng.bottomRow(); ++i)
        {
            if(ui->tableWidget->item(i,0)->data(102).toBool() != checkstat)
            {
                ui->downloadLater->setCheckState(Qt::PartiallyChecked);
                connect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
                return;
            }
        }
    connect(ui->downloadLater,SIGNAL(stateChanged(int)),this,SLOT(setDownloadLater()));
}

void ImportDialog::setCategory(int id)
{
    QList<QTableWidgetSelectionRange> rngs = ui->tableWidget->selectedRanges();
    if(rngs.isEmpty())
        return;

    int catid = ui->categoryBox->itemData(id).toInt();
    QTableWidgetSelectionRange rng;
    foreach(rng,rngs)
        for(int i = rng.topRow(); i <= rng.bottomRow(); ++i)
        {
            QTableWidgetItem *itm = ui->tableWidget->item(i,0);
            itm->setData(100,catid);
            //itm->setData(101,ui->locationEdit->text());
        }
}

void ImportDialog::setDownloadLater()
{
    QList<QTableWidgetSelectionRange> rngs = ui->tableWidget->selectedRanges();
    if(rngs.isEmpty())
        return;

    QTableWidgetSelectionRange rng;
    foreach(rng,rngs)
        for(int i = rng.topRow(); i <= rng.bottomRow(); ++i)
        {
            QTableWidgetItem *itm = ui->tableWidget->item(i,0);
            itm->setData(102,ui->downloadLater->isChecked());
        }
}

void ImportDialog::showDirDialog()
{
    QList<QTableWidgetSelectionRange> rngs = ui->tableWidget->selectedRanges();
    if(rngs.isEmpty())
        return;

    QString dir = QFileDialog::getExistingDirectory(this,tr("Выбор директории"),ui->locationEdit->text(), QFileDialog::ShowDirsOnly | QFileDialog::DontUseNativeDialog);
    if(dir.isEmpty())
        return;

    QTableWidgetSelectionRange rng;
    foreach(rng,rngs)
        for(int i = rng.topRow(); i <= rng.bottomRow(); ++i)
        {
            QTableWidgetItem *itm = ui->tableWidget->item(i,0);
            itm->setData(101,dir);
        }
    ui->locationEdit->setText(dir);
}

void ImportDialog::addTasks()
{
    QList<QTableWidgetSelectionRange> rngs = ui->tableWidget->selectedRanges();
    if(rngs.isEmpty())
        return;

    QTableWidgetSelectionRange rng;
    foreach(rng,rngs)
        for(int i = rng.topRow(); i <= rng.bottomRow(); ++i)
        {
            QTableWidgetItem *itm = ui->tableWidget->item(i,0);
            if(addTask(itm))
                ui->tableWidget->hideRow(itm->row());
            qApp->processEvents(QEventLoop::AllEvents,40);
        }
    emit addedNewTask();
    showWarning();

    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if(!btn || btn == ui->pushButton)
        return;
    close();
}

bool ImportDialog::addTask(QTableWidgetItem *itm)
{
    QSqlQuery qr(mydb);

    qr.prepare("SELECT count(*) FROM tasks WHERE url=:url;");
    qr.bindValue("url", itm->text());
    if(!qr.exec())
    {
        //тут запись в журнал ошибок
        qDebug()<<"void ImportDialog::addTask(1):"<<qr.executedQuery()<<" Error: "<<qr.lastError().text();
        close();
        return false;
    }
    qr.next();

    if(qr.value(0).toInt() > 0)
    {
        if(tasks_exists.isEmpty())
            tasks_exists= itm->text();
        else tasks_exists += QString("\n\n%1").arg(itm->text());

        return false;
    }

    int catId = itm->data(100).toInt();

    QString newpath = itm->data(101).toString() != "" ? itm->data(101).toString() : dirs.value(catId);
    QDir().mkpath(newpath);
    QDateTime dtime(QDateTime::currentDateTime());
    QUrl curl = QUrl::fromEncoded(itm->text().toUtf8());
    QFileInfo flinfo(curl.toString(QUrl::RemoveQuery | QUrl::RemoveFragment));
    QString flname;
    flname = (flinfo.fileName() != QString() ? flinfo.fileName() + "." + dtime.toString("yyyyMMddhhmmss") + ".rldr" : "noname.html");
    flname = newpath + "/" + flname;

    qr.clear();
    qr.prepare("INSERT INTO tasks(url,filename,datecreate,tstatus,categoryid,priority) VALUES(:url,:filename,:datecreate,:tstatus,:categoryid,:priority);");
    qr.bindValue("url", itm->text());
    qr.bindValue("filename",flname);
    qr.bindValue("datecreate",dtime.toString("yyyy-MM-ddThh:mm:ss"));
    if(itm->data(102).toBool()) qr.bindValue("tstatus",0);
    else qr.bindValue("tstatus",-100);
    qr.bindValue("categoryid",catId);
    qr.bindValue("priority",2);

    if(!qr.exec())
    {
        //тут запись в журнал ошибок
        qDebug()<<"void ImportDialog::addTask(2): Error: "<<qr.lastError().text();
        close();
        return false;
    }

    return true;
}

void ImportDialog::showWarning() const
{
    if(tasks_exists.isEmpty())
        return;

    QMessageBox *dlg = new QMessageBox((QWidget*)parent());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setIcon(QMessageBox::Warning);
    dlg->setModal(true);
    dlg->setWindowTitle(tr("Предупреждение"));
    dlg->setText(tr("Некоторые URL уже присутствуют в списке заданий.\nДля уточнения списка совпавших URL кликните по кнопке \"Показать подробности...\""));
    dlg->setDetailedText(tasks_exists);
    dlg->show();
}

void ImportDialog::import()
{
    if(protocols.isEmpty())
        return;

    ImportMaster *importmaster = new ImportMaster(this);
    connect(importmaster,SIGNAL(finished()),importmaster,SLOT(deleteLater()));
    connect(importmaster,SIGNAL(foundString(QString)),this,SLOT(addUrl(QString)));
    //connect(importmaster,SIGNAL(totalStringLoaded(qint64)),this,SLOT(readNewLine(qint64)));
    QString proto;
    foreach(proto,protocols)
        importmaster->addProtocol(proto);

    importmaster->import(imp_files);
}

void ImportDialog::addUrl(const QString &url)
{
    if(contains(url))
        return;

    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
    QTableWidgetItem *itm = new QTableWidgetItem(url);
    itm->setData(Qt::ToolTipRole,url);
    itm->setData(100,getCategory(url));
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,0,itm);
    ui->foundURLs->setText(QString::number(++fndurl));
}

void ImportDialog::initialize()
{
    if(!parent())setWindowIcon(QIcon(":/appimages/trayicon.png"));
    setAttribute(Qt::WA_DeleteOnClose);
    fndurl = 0;
    ui->tableWidget->horizontalHeader()->hide();
    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
#if QT_VERSION < 0x050000
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
#else
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
    ui->tableWidget->setColumnCount(1);
}
