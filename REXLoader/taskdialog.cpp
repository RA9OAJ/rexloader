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

void TaskDialog::setSourceData(TItemModel *model, QModelIndex index, const QHash<int,LoaderInterface*> &pluglist, const QHash<int,int> &list)
{
    mdl = model;
    idx = index;
    ldr = &pluglist;
    lst = &list;

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
        int value = (mdl->data(mdl->index(idx.row(),5),100).toLongLong()) ? mdl->data(mdl->index(idx.row(),4),100).toLongLong() * 100 / mdl->data(mdl->index(idx.row(),5),100).toLongLong() : 0;
        ui->progressBar->setValue(value);
        ui->loadLabel->setText(load.value(0) + load.value(1));
        ui->timeLabel->setText(mdl->data(mdl->index(idx.row(),6),Qt::DisplayRole).toString());
        ui->leftLabel->setText(mdl->data(mdl->index(idx.row(),15),Qt::DisplayRole).toString());
        int id = mdl->data(mdl->index(idx.row(),0),100).toInt();
        int task_id = lst->value(id)%100;
        int proto_id = lst->value(id)/100;
        int sect_cnt = 0;

        if(ldr->contains(proto_id))
                   sect_cnt = ldr->value(proto_id)->countSectionTask(task_id);

        int status = mdl->data(mdl->index(idx.row(),9),100).toInt();
        switch(status)
        {
        case LInterface::ERROR_TASK: ui->statusLabel->setText(QString("<font color='red'>%1</font>").arg(tr("Ошибка"))); break;
        case LInterface::STOPPING:
        case LInterface::ON_PAUSE: ui->statusLabel->setText(tr("Приостановлено")); break;
        case -100: ui->statusLabel->setText(tr("Ожидание")); break;
        case LInterface::FINISHED: ui->statusLabel->setText(tr("Завершено")); break;
        case LInterface::SEND_QUERY: ui->statusLabel->setText(QString("%1 (%2)").arg(tr("Посылка запроса"),QString::number(sect_cnt ? sect_cnt : 1))); break;
        case LInterface::ACCEPT_QUERY: ui->statusLabel->setText(QString("%1 (%2)").arg(tr("Запрос принят"),QString::number(sect_cnt))); break;
        case LInterface::REDIRECT:
        case LInterface::ON_LOAD: ui->statusLabel->setText(tr("Закачивается")); break;

        default: ui->statusLabel->setText(ldr->value(proto_id)->statusString(status)); break;
        }

        ui->sectionLabel->setText(sect_cnt == 0 ? "":QString::number(sect_cnt));
    }

    QTimer::singleShot(500,this,SLOT(scheduler()));
}
