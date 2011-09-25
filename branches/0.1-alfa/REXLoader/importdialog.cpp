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

ImportDialog::ImportDialog(const QString &dir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

    mydb = QSqlDatabase::database();

    initialize();
}

ImportDialog::ImportDialog(const QString &dir, QSqlDatabase &db_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImportDialog)
{
    ui->setupUi(this);

    mydb = db_;

    initialize();
}

ImportDialog::~ImportDialog()
{
    delete ui;
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
        else if(qr.value(1).toString() == "#archives"){cattitle = tr("Archives"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#apps"){cattitle = tr("Applications"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#audio"){cattitle = tr("Audio"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#video"){cattitle = tr("Video"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#other"){cattitle = tr("All Downloads"); otherId = qr.value(0).toInt();dirs.insert(qr.value(0).toInt(),downDir);}
        else {cattitle = qr.value(1).toString(); dirs.insert(qr.value(0).toInt(),qr.value(2).toString());}

        ui->categoryBox->addItem(cattitle, qr.value(0).toInt());
    }
    ui->categoryBox->setCurrentIndex(ui->categoryBox->findData(QVariant(otherId)));
}

void ImportDialog::initialize()
{
    if(!parent())setWindowIcon(QIcon(":/appimages/trayicon.png"));
    setAttribute(Qt::WA_DeleteOnClose);
}
