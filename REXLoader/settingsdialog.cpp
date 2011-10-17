/*
Project: REXLoader (Downloader), Source file: settingsdialog.cpp
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

#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QDebug>

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
    ui->downDir->setText(QDir::home().path()+"/"+tr("Downloads"));

    connect(ui->listWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(selectSubSettings()));
    connect(ui->proxyCheckBox,SIGNAL(toggled(bool)),ui->groupBox,SLOT(setEnabled(bool)));
    connect(ui->buttonBox->button(QDialogButtonBox::Apply),SIGNAL(released()),this,SLOT(applySets()));
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel),SIGNAL(released()),this,SLOT(close()));
    connect(ui->buttonBox->button(QDialogButtonBox::Ok),SIGNAL(released()),this,SLOT(applyAndClose()));
    connect(ui->browseButton,SIGNAL(released()),this,SLOT(showFileDialog()));

    ui->networkBox->setVisible(false);
    ui->downloadsBox->setVisible(false);
    resize(size().width(),220);
    applySets();
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

    switch(ui->listWidget->currentRow())
    {
    case 0:
        ui->generalBox->setVisible(true);
        ui->networkBox->setVisible(false);
        ui->downloadsBox->setVisible(false);
        break;
    case 1:
        ui->networkBox->setVisible(true);
        ui->generalBox->setVisible(false);
        ui->downloadsBox->setVisible(false);
        break;
    case 2:
        ui->downloadsBox->setVisible(true);
        ui->networkBox->setVisible(false);
        ui->generalBox->setVisible(false);
        break;
    default: ui->generalBox->setVisible(false); break;
    }

    item = ui->listWidget->item(ui->listWidget->currentRow());
    font = item->font();
    font.setBold(true);
    item->setFont(font);
    last_row = ui->listWidget->currentRow();
}

void SettingsDialog::show()
{
    if(!isVisible())
    {
        QDialog::show();

        QDesktopWidget ds;
        QRect desktop = ds.availableGeometry();
        QPoint top_left = QPoint((desktop.bottomRight().x()-size().width())/2,(desktop.bottomRight().y()-size().height())/2);
        move(top_left);
    }
    else activateWindow();
}

void SettingsDialog::applySets()
{
    sets.insert("show_logo",ui->checkBoxLogo->isChecked());
    sets.insert("start_minimized",ui->checkBoxMinimize->isChecked());
    sets.insert("animate_tray",ui->animateTrayBox->isChecked());
    sets.insert("check_updates",ui->checkBoxUpdates->isChecked());
    sets.insert("restore_passwords",ui->checkBoxPasswords->isChecked());
    sets.insert("scan_clipboard",ui->scanClipboard->isChecked());
    sets.insert("notshow_adding",ui->notShowAddingDialog->isChecked());
    sets.insert("user_agent",ui->userAgent->text());

    sets.insert("s_hight",(qint64)ui->spinHight->value());
    sets.insert("s_normal",(qint64)ui->spinNormal->value());
    sets.insert("s_low",(qint64)ui->spinLow->value());
    sets.insert("s_vlow",(qint64)ui->spinVLow->value());
    sets.insert("timeout_interval",ui->connectionTimeout->value());

    sets.insert("proxy_enable",ui->proxyCheckBox->isChecked());
    sets.insert("proxy_address",ui->proxyAddress->text());
    sets.insert("proxy_port",ui->proxyPort->value());
    sets.insert("enable_sockss",ui->sockssCheck->isChecked());
    sets.insert("proxy_user",ui->proxyUser->text());
    sets.insert("proxy_password",ui->proxyPassword->text());

    sets.insert("down_dir",ui->downDir->text());
    sets.insert("max_number_tasks",ui->maxTasks->currentIndex()+1);
    sets.insert("max_number_sections",ui->maxSections->currentIndex()+1);
    sets.insert("enable_ignore_errors",ui->checkBoxIgnoreErrors->isChecked());
    sets.insert("max_number_errors",ui->spinBoxMaxErrors->value());

    emit newSettings();
}

void SettingsDialog::cancelSets()
{
    ui->checkBoxLogo->setChecked(sets.value("show_logo").toBool());
    ui->checkBoxMinimize->setChecked(sets.value("start_minimized").toBool());
    ui->animateTrayBox->setChecked(sets.value("animate_tray").toBool());
    ui->checkBoxUpdates->setChecked(sets.value("check_updates").toBool());
    ui->checkBoxPasswords->setChecked(sets.value("restore_passwords").toBool());
    ui->scanClipboard->setChecked(sets.value("scan_clipboard").toBool());
    ui->notShowAddingDialog->setChecked(sets.value("notshow_adding").toBool());
    ui->userAgent->setText(sets.value("user_agent").toString());

    ui->spinHight->setValue(sets.value("s_hight").toLongLong());
    ui->spinNormal->setValue(sets.value("s_normal").toLongLong());
    ui->spinLow->setValue(sets.value("s_low").toLongLong());
    ui->spinVLow->setValue(sets.value("s_vlow").toLongLong());
    ui->connectionTimeout->setValue(sets.value("timeout_interval").toInt());

    ui->proxyCheckBox->setChecked(sets.value("proxy_enable").toBool());
    ui->proxyAddress->setText(sets.value("proxy_address").toString());
    ui->proxyPort->setValue(sets.value("proxy_port").toInt());
    ui->sockssCheck->setChecked(sets.value("enable_sockss").toBool());
    ui->proxyUser->setText(sets.value("proxy_user").toString());
    ui->proxyPassword->setText(sets.value("proxy_password").toString());

    ui->downDir->setText(sets.value("down_dir").toString());
    ui->maxTasks->setCurrentIndex(sets.value("max_number_tasks").toInt()-1);
    ui->maxSections->setCurrentIndex(sets.value("max_number_sections").toInt()-1);
    ui->checkBoxIgnoreErrors->setChecked(sets.value("enable_ignore_errors").toBool());
    ui->spinBoxMaxErrors->setValue(sets.value("max_number_errors").toInt());
}

QList<QString> SettingsDialog::keys() const
{
    return sets.keys();
}

QVariant SettingsDialog::value(const QString &key) const
{
    return sets.value(key);
}

void SettingsDialog::closeEvent(QCloseEvent *event)
{
    cancelSets();
    QDialog::closeEvent(event);
}

void SettingsDialog::applyAndClose()
{
    applySets();
    accept();
}

void SettingsDialog::setDownDir(const QString &dir)
{
    ui->downDir->setText(dir);
}

void SettingsDialog::showFileDialog()
{
    QFileDialog *dlg = new QFileDialog(this);
    dlg->setDirectory(ui->downDir->text());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFileMode(QFileDialog::DirectoryOnly);
    dlg->setWindowTitle(tr("Select directory"));
    dlg->setOption(QFileDialog::DontUseNativeDialog);
    connect(dlg,SIGNAL(fileSelected(QString)),this,SLOT(setDownDir(QString)));
    dlg->show();
}

void SettingsDialog::setSettingAttribute(const QString &key, const QVariant &value)
{
    sets.insert(key,value);
}

void SettingsDialog::updateInterface()
{
    cancelSets();
    emit newSettings();
}
