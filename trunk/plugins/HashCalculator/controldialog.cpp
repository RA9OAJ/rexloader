/*
Copyright (C) 2012-2013  Alexey Schukin

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

#include "controldialog.h"
#include "ui_controldialog.h"


ControlDialog::ControlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ControlDialog)
{
    ui->setupUi(this);
    mp_hash_calc_thread = new HashCalculatorThread(this);
    QStringList headers;
    headers << tr("Имя файла") << "md5" << "sha1";
    row = 0;
    ui->table->setHorizontalHeaderLabels(headers);
    ui->table->setColumnCount(3);
    connect(mp_hash_calc_thread, SIGNAL(progress(QString, int)),
            this, SLOT(progress(QString,int)), Qt::QueuedConnection);
    connect(mp_hash_calc_thread, SIGNAL(calcFinished(QString,QString,QString)),
            this, SLOT(calcFinished(QString,QString,QString)), Qt::QueuedConnection);
}

ControlDialog::~ControlDialog()
{
    delete ui;
}

void ControlDialog::setFileNames(const QStringList &file_name)
{
    mp_hash_calc_thread->setFileNames(file_name);
    row = 0;
    ui->table->clearContents();
    ui->table->setRowCount(file_name.size());
}

void ControlDialog::progress(QString file_name, int percent)
{
    Q_UNUSED(file_name)
    ui->progressBar->setValue(percent);
    ui->file_name->setText(file_name);
}

void ControlDialog::calcFinished(QString file_name, QString md5_result, QString sha1_result)
{
    QTableWidgetItem *name = new QTableWidgetItem(file_name);
    QTableWidgetItem *md5  = new QTableWidgetItem(md5_result);
    QTableWidgetItem *sha1 = new QTableWidgetItem(sha1_result);
    ui->table->setItem(row, 0, name);
    ui->table->setItem(row, 1, md5);
    ui->table->setItem(row, 2, sha1);
    row++;
    ui->table->resizeColumnsToContents();
//    qDebug() << file_name;
//    qDebug() << md5_result;
//    qDebug() << sha1_result;
}

void ControlDialog::slotClose()
{
    mp_hash_calc_thread->stop();
    reject();
}

int ControlDialog::exec()
{
    mp_hash_calc_thread->start();
    return QDialog::exec();
}
