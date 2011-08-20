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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    setWindowTitle(parent->windowTitle() + " - " + tr("Settings"));
    QList<int> sz;
    sz << 50 << 200;
    ui->splitter->setSizes(sz);
    last_row = 0;

    connect(ui->listWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(selectSubSettings()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::selectSubSettings()
{
    QListWidgetItem *item = ui->listWidget->item(last_row);//ui->listWidget->item(ui->listWidget->currentRow());
    QFont font = item->font();
    font.setBold(false);
    item->setFont(font);

    item = ui->listWidget->item(ui->listWidget->currentRow());
    font = item->font();
    font.setBold(true);
    item->setFont(font);
    last_row = ui->listWidget->currentRow();
}
