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
}

void ImportDialog::initialize()
{
    if(!parent())setWindowIcon(QIcon(":/appimages/trayicon.png"));
}
