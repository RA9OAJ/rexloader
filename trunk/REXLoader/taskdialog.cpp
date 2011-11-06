/*
<one line to give the program's name and a brief idea of what it does.>
Copyright (C) <year>  <name of author>

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

#include "taskdialog.h"
#include "ui_taskdialog.h"

TaskDialog::TaskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);

    QTimer::singleShot(0,this,SLOT(scheduler()));
}

TaskDialog::~TaskDialog()
{
    delete ui;
}

void TaskDialog::setSourceData(TItemModel *model, QModelIndex index, LoaderInterface *loader)
{
    mdl = model;
    idx = index;
    ldr = loader;

    if(mdl)
    {
        ui->lineEdit->setText(mdl->data(mdl->index(idx.row(),1),100).toString());
        ui->comboBox->setCurrentIndex(mdl->data(mdl->index(idx.row(),13),100).toInt());
    }
}

void TaskDialog::deleteThisTask(QModelIndex index)
{
    if(index == idx)
        close();
}

void TaskDialog::scheduler()
{
    if(mdl)
    {
        ui->sizeLabel->setText(mdl->data(mdl->index(idx.row(),5),Qt::DisplayRole).toString());
        QStringList load = TItemModel::sizeForHumans(mdl->data(mdl->index(idx.row(),4),100).toLongLong());
        ui->loadLabel->setText(load.value(0) + load.value(1));
        ui->timeLabel->setText(mdl->data(mdl->index(idx.row(),6),Qt::DisplayRole).toString());
        ui->leftLabel->setText(mdl->data(mdl->index(idx.row(),15),Qt::DisplayRole).toString());
    }

    QTimer::singleShot(1000,this,SLOT(scheduler()));
}
