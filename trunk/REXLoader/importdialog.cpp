/*
Project: REXLoader (Downloader), Source file: importdialog.cpp
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

void ImportDialog::selectAll()
{
    QTableWidgetSelectionRange rng(0,0,ui->tableWidget->rowCount()-1,0);
    ui->tableWidget->setRangeSelected(rng,true);
}

void ImportDialog::deselectAll()
{
    QTableWidgetSelectionRange rng(0,0,ui->tableWidget->rowCount()-1,0);
    ui->tableWidget->setRangeSelected(rng,false);
}

void ImportDialog::invertSelection()
{
    QList<QTableWidgetSelectionRange> rngs = ui->tableWidget->selectedRanges();
    selectAll();
    QTableWidgetSelectionRange rng;
    foreach(rng,rngs)
        ui->tableWidget->setRangeSelected(rng,false);
}

void ImportDialog::locationSelected(int id)
{
    ui->locationEdit->setText(dirs.value(ui->categoryBox->itemData(id).toInt()));
}

void ImportDialog::import()
{
    ImportMaster *importmaster = new ImportMaster(this);
    connect(importmaster,SIGNAL(finished()),importmaster,SLOT(deleteLater()));
    connect(importmaster,SIGNAL(foundString(QString)),this,SLOT(addUrl(QString)));
    connect(importmaster,SIGNAL(totalStringLoaded(qint64)),this,SLOT(readNewLine(qint64)));
    importmaster->import(imp_files);
}

void ImportDialog::addUrl(const QString &url)
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
    QTableWidgetItem *itm = new QTableWidgetItem(url);
    itm->setData(Qt::ToolTipRole,url);
    ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,0,itm);
    ui->foundURLs->setText(QString::number(++fndurl));
}

void ImportDialog::readNewLine(qint64 cnt)
{
    ui->totalStrings->setText(QString::number(cnt));
}

void ImportDialog::initialize()
{
    if(!parent())setWindowIcon(QIcon(":/appimages/trayicon.png"));
    setAttribute(Qt::WA_DeleteOnClose);
    fndurl = 0;
    ui->tableWidget->horizontalHeader()->hide();
    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->hideColumn(1);
}
