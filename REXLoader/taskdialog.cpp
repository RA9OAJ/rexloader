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

#include "taskdialog.h"
#include "ui_taskdialog.h"

int TaskDialog::obj_cnt = 0;

TaskDialog::TaskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskDialog)
{
    ++obj_cnt;
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->startButton,SIGNAL(released()),this,SLOT(pressAnaliser()));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(selectPriority(int)));
    connect(ui->settingsButton,SIGNAL(released()),this,SLOT(showTaskSettings()));

    QTimer::singleShot(0,this,SLOT(scheduler()));
    moveToCenter();
}

TaskDialog::TaskDialog(const QString &dir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskDialog)
{
    ++obj_cnt;
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->startButton,SIGNAL(released()),this,SLOT(pressAnaliser()));
    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(selectPriority(int)));
    connect(ui->settingsButton,SIGNAL(released()),this,SLOT(showTaskSettings()));

    QTimer::singleShot(0,this,SLOT(scheduler()));
    moveToCenter();

    downDir = dir;
}

TaskDialog::~TaskDialog()
{
    delete ui;

    --obj_cnt;
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
        QFileInfo flinfo(model->data(model->index(index.row(),3),Qt::DisplayRole).toString());
        setWindowTitle(windowTitle() + QString(" - %1").arg(flinfo.fileName()));
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
        ui->leftLabel->setText(mdl->data(mdl->index(idx.row(),18),Qt::DisplayRole).toString());
        int id = mdl->data(mdl->index(idx.row(),0),100).toInt();
        int task_id = lst->value(id)%100;
        int proto_id = lst->value(id)/100;
        int sect_cnt = 0;

        int status = mdl->data(mdl->index(idx.row(),9),100).toInt();

        if(ldr->contains(proto_id))
        {
            sect_cnt = ldr->value(proto_id)->countSectionTask(task_id);
            if(sect_cnt && status == LInterface::ON_LOAD)
            {
                ui->acceptIcon->setText(ldr->value(proto_id)->acceptRanges(task_id) ? tr("Есть") : tr("Нет"));
                ui->acceptIcon->setPixmap(ldr->value(proto_id)->acceptRanges(task_id) ? QPixmap(":/appimages/yes_16x16.png") : QPixmap(":/appimages/no_16x16.png"));
            }
        }

        switch(status)
        {
        case LInterface::ERROR_TASK:
        {
            ui->statusLabel->setText(QString("<font color='red'>%1</font>").arg(tr("Ошибка")));
            ui->startButton->setText(tr("Запустить"));
            ui->startButton->setIcon(QIcon(":/appimages/start_24x24.png"));
            ui->startButton->setEnabled(true);
            ui->comboBox->setEnabled(true);
            break;
        }
        case LInterface::STOPPING:
        case LInterface::ON_PAUSE:
        {
            ui->statusLabel->setText(tr("Приостановлено"));
            ui->startButton->setText(tr("Запустить"));
            ui->startButton->setIcon(QIcon(":/appimages/start_24x24.png"));
            ui->startButton->setEnabled(true);
            ui->comboBox->setEnabled(true);
            break;
        }
        case -100:
        {
            ui->statusLabel->setText(tr("Ожидание"));
            ui->startButton->setText(tr("Приостановить"));
            ui->startButton->setIcon(QIcon(":/appimages/pause_24x24.png"));
            ui->startButton->setEnabled(true);
            ui->comboBox->setEnabled(true);
            break;
        }
        case LInterface::FINISHED:
        {
            ui->statusLabel->setText(tr("Завершено"));
            ui->startButton->setText(tr("Перезакачать"));
            ui->startButton->setIcon(QIcon());
            ui->startButton->setEnabled(false);
            ui->comboBox->setEnabled(false);
            break;
        }
        case LInterface::SEND_QUERY:
        {
            ui->statusLabel->setText(QString("%1 (%2)").arg(tr("Посылка запроса"),QString::number(sect_cnt ? sect_cnt : 1)));
            ui->startButton->setText(tr("Приостановить"));
            ui->startButton->setIcon(QIcon(":/appimages/pause_24x24.png"));
            ui->startButton->setEnabled(true);
            ui->comboBox->setEnabled(true);
            break;
        }
        case LInterface::ACCEPT_QUERY:
        {
            ui->statusLabel->setText(QString("%1 (%2)").arg(tr("Запрос принят"),QString::number(sect_cnt)));
            ui->startButton->setText(tr("Приостановить"));
            ui->startButton->setIcon(QIcon(":/appimages/pause_24x24.png"));
            ui->startButton->setEnabled(true);
            ui->comboBox->setEnabled(true);
            break;
        }
        case LInterface::REDIRECT:
        case LInterface::ON_LOAD:
        {
            ui->statusLabel->setText(tr("Закачивается"));
            ui->startButton->setText(tr("Приостановить"));
            ui->startButton->setIcon(QIcon(":/appimages/pause_24x24.png"));
            ui->startButton->setEnabled(true);
            ui->comboBox->setEnabled(true);
            break;
        }

        default:
            if(proto_id)
                ui->statusLabel->setText(ldr->value(proto_id)->statusString(status)); break;
        }

        ui->sectionLabel->setText(sect_cnt == 0 ? "":QString::number(sect_cnt));
    }

    QTimer::singleShot(500,this,SLOT(scheduler()));
}

void TaskDialog::pressAnaliser()
{
    if(mdl)
    {
        int id = mdl->data(mdl->index(idx.row(),0),100).toInt();

        int status = mdl->data(mdl->index(idx.row(),9),100).toInt();
        switch(status)
        {
        case LInterface::ERROR_TASK:
        case LInterface::STOPPING:
        case LInterface::ON_PAUSE:  emit startTask(id); break;
        case -100:  break;
        case LInterface::FINISHED:  emit redownloadTask(id); break;
        case LInterface::SEND_QUERY:
        case LInterface::ACCEPT_QUERY:
        case LInterface::REDIRECT:
        case LInterface::ON_LOAD: emit stopTask(id); break;

        default: break;
        }
        ui->startButton->setDisabled(true);
    }
}

void TaskDialog::moveToCenter()
{
    QDesktopWidget ds;
    QRect desktop = ds.availableGeometry();
    QPoint top_left = QPoint((desktop.bottomRight().x()-size().width())/2+20*(obj_cnt-1),(desktop.bottomRight().y()-size().height())/2+20*(obj_cnt-1));
    move(top_left);
}

void TaskDialog::selectPriority(int cur_index)
{
    if(mdl)
    {
        int id = mdl->data(mdl->index(idx.row(),0),100).toInt();
        emit setPriority(id,cur_index);
    }
}

void TaskDialog::showTaskSettings()
{
    AddTaskDialog *dlg = new AddTaskDialog(downDir,this);
    connect(dlg,SIGNAL(taskUpdateStart(int)),parent(),SLOT(startUpdateTaskProc(int)));
    connect(dlg,SIGNAL(taskUpdateEnd(int)),parent(),SLOT(endUpdateTaskProc(int)));
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setUpdateMode(idx);
    dlg->show();
}
