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

#include "taskschedulerdialog.h"
#include "ui_taskschedulerdialog.h"

TaskSchedulerDialog::TaskSchedulerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TaskSchedulerDialog)
{
    ui->setupUi(this);
}

TaskSchedulerDialog::~TaskSchedulerDialog()
{
    delete ui;
}
