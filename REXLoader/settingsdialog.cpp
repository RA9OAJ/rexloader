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

    if(!parent)setWindowIcon(QIcon(":/appimages/trayicon.png"));
    else setWindowTitle(parent->windowTitle() + " - " + tr("Настройки"));
    QList<int> sz;
    sz << 50 << 200;
    ui->splitter->setSizes(sz);
    last_row = 0;
    ui->downDir->setText(QDir::home().path()+"/"+tr("Загрузки"));
    ui->logDir->setText(QDir::home().path()+"./rexloader/logs");

    connect(ui->listWidget,SIGNAL(clicked(QModelIndex)),this,SLOT(selectSubSettings()));
    connect(ui->proxyCheckBox,SIGNAL(toggled(bool)),ui->groupBox,SLOT(setEnabled(bool)));
    connect(ui->buttonBox->button(QDialogButtonBox::Apply),SIGNAL(released()),this,SLOT(applySets()));
    connect(ui->buttonBox->button(QDialogButtonBox::Cancel),SIGNAL(released()),this,SLOT(close()));
    connect(ui->buttonBox->button(QDialogButtonBox::Ok),SIGNAL(released()),this,SLOT(applyAndClose()));
    connect(ui->browseButton,SIGNAL(released()),this,SLOT(showFileDialog()));
    connect(ui->logBrowseButton,SIGNAL(released()),this,SLOT(showFileDialog()));

    connect(ui->onPauseColor,SIGNAL(colorSelected(QColor)),ui->onPauseFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->onPauseFontColor,SIGNAL(colorSelected(QColor)),ui->onPauseFont,SLOT(setFontColor(QColor)));
    connect(ui->onQueueColor,SIGNAL(colorSelected(QColor)),ui->onQueueFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->onQueueFontColor,SIGNAL(colorSelected(QColor)),ui->onQueueFont,SLOT(setFontColor(QColor)));
    connect(ui->onLoadColor,SIGNAL(colorSelected(QColor)),ui->onLoadFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->onLoadFontColor,SIGNAL(colorSelected(QColor)),ui->onLoadFont,SLOT(setFontColor(QColor)));
    connect(ui->onErrorColor,SIGNAL(colorSelected(QColor)),ui->onErrorFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->onErrorFontColor,SIGNAL(colorSelected(QColor)),ui->onErrorFont,SLOT(setFontColor(QColor)));
    connect(ui->onFinishColor,SIGNAL(colorSelected(QColor)),ui->onFinishFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->onFinishFontColor,SIGNAL(colorSelected(QColor)),ui->onFinishFont,SLOT(setFontColor(QColor)));
    connect(ui->fontsColorsReset,SIGNAL(released()),this,SLOT(resetFontsColors()));

    connect(ui->infoColor,SIGNAL(colorSelected(QColor)),ui->infoFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->infoFontColor,SIGNAL(colorSelected(QColor)),ui->infoFont,SLOT(setFontColor(QColor)));
    connect(ui->warningColor,SIGNAL(colorSelected(QColor)),ui->warningFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->warningFontColor,SIGNAL(colorSelected(QColor)),ui->warningFont,SLOT(setFontColor(QColor)));
    connect(ui->errorColor,SIGNAL(colorSelected(QColor)),ui->errorFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->errorFontColor,SIGNAL(colorSelected(QColor)),ui->errorFont,SLOT(setFontColor(QColor)));
    connect(ui->outColor,SIGNAL(colorSelected(QColor)),ui->outFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->outFontColor,SIGNAL(colorSelected(QColor)),ui->outFont,SLOT(setFontColor(QColor)));
    connect(ui->inColor,SIGNAL(colorSelected(QColor)),ui->inFont,SLOT(setBackgroundColor(QColor)));
    connect(ui->inFontColor,SIGNAL(colorSelected(QColor)),ui->inFont,SLOT(setFontColor(QColor)));
    connect(ui->logFontColorReset,SIGNAL(released()),this,SLOT(resetFontsColors()));

    resetFontsColors();

    ui->networkBox->setVisible(false);
    ui->downloadsBox->setVisible(false);
    ui->interfaceBox->setVisible(false);
    ui->logBox->setVisible(false);
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
        ui->downloadsBox->setVisible(false);
        ui->networkBox->setVisible(false);
        ui->generalBox->setVisible(true);
        ui->interfaceBox->setVisible(false);
        ui->logBox->setVisible(false);
        break;
    case 1:
        ui->downloadsBox->setVisible(false);
        ui->generalBox->setVisible(false);
        ui->networkBox->setVisible(true);
        ui->interfaceBox->setVisible(false);
        ui->logBox->setVisible(false);
        break;
    case 2:
        ui->networkBox->setVisible(false);
        ui->generalBox->setVisible(false);
        ui->downloadsBox->setVisible(true);
        ui->interfaceBox->setVisible(false);
        ui->logBox->setVisible(false);
        break;
    case 3:
        ui->networkBox->setVisible(false);
        ui->generalBox->setVisible(false);
        ui->downloadsBox->setVisible(false);
        ui->interfaceBox->setVisible(true);
        ui->logBox->setVisible(false);
        break;
    case 5:
        ui->networkBox->setVisible(false);
        ui->generalBox->setVisible(false);
        ui->downloadsBox->setVisible(false);
        ui->interfaceBox->setVisible(false);
        ui->logBox->setVisible(true);
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
    sets.insert("attempt_interval",ui->spinBoxAttemptInterval->value());

    sets.insert("show_taskdialog",ui->showTaskDialog->isChecked());

    sets.insert("on_pause_color",ui->onPauseColor->currentColor());
    sets.insert("on_pause_font",ui->onPauseFont->font());
    sets.insert("on_pause_font_color",ui->onPauseFontColor->currentColor());
    sets.insert("on_load_color",ui->onLoadColor->currentColor());
    sets.insert("on_load_font",ui->onLoadFont->font());
    sets.insert("on_load_font_color",ui->onLoadFontColor->currentColor());
    sets.insert("on_queue_color",ui->onQueueColor->currentColor());
    sets.insert("on_queue_font",ui->onQueueFont->font());
    sets.insert("on_queue_font_color",ui->onQueueFontColor->currentColor());
    sets.insert("on_error_color",ui->onErrorColor->currentColor());
    sets.insert("on_error_font",ui->onErrorFont->font());
    sets.insert("on_error_font_color",ui->onErrorFontColor->currentColor());
    sets.insert("on_finish_color",ui->onFinishColor->currentColor());
    sets.insert("on_finish_font",ui->onFinishFont->font());
    sets.insert("on_finish_font_color",ui->onFinishFontColor->currentColor());
    sets.insert("table_word_wrap",ui->tableWordWrap->isChecked());

    sets.insert("log_dir",ui->logDir->text());
    sets.insert("log_autosave",ui->logAutisave->isChecked());
    sets.insert("log_life_time",ui->logLifeTime->value());
    sets.insert("log_info_color",ui->infoColor->currentColor());
    sets.insert("log_info_font",ui->infoFont->font());
    sets.insert("log_info_font_color",ui->infoFontColor->currentColor());
    sets.insert("log_warning_color",ui->warningColor->currentColor());
    sets.insert("log_warning_font",ui->warningFont->font());
    sets.insert("log_warning_font_color",ui->warningFontColor->currentColor());
    sets.insert("log_error_color",ui->errorColor->currentColor());
    sets.insert("log_error_font",ui->errorFont->font());
    sets.insert("log_error_font_color",ui->errorFontColor->currentColor());
    sets.insert("log_out_color",ui->outColor->currentColor());
    sets.insert("log_out_font",ui->outFont->font());
    sets.insert("log_out_fonst_color",ui->outFontColor->currentColor());
    sets.insert("log_in_color",ui->inColor->currentColor());
    sets.insert("log_in_font",ui->inFont->font());
    sets.insert("log_in_font_color",ui->inFontColor->currentColor());

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
    ui->spinBoxAttemptInterval->setValue(sets.value("attempt_interval").toInt());

    ui->showTaskDialog->setChecked(sets.value("show_taskdialog").toBool());

    ui->onPauseColor->setColor(sets.value("on_pause_color").value<QColor>());
    ui->onPauseFont->setFont(sets.value("on_pause_font").value<QFont>());
    ui->onPauseFontColor->setColor(sets.value("on_pause_font_color").value<QColor>());
    ui->onLoadColor->setColor(sets.value("on_load_color").value<QColor>());
    ui->onLoadFont->setFont(sets.value("on_load_font").value<QFont>());
    ui->onLoadFontColor->setColor(sets.value("on_load_font_color").value<QColor>());
    ui->onQueueColor->setColor(sets.value("on_queue_color").value<QColor>());
    ui->onQueueFont->setFont(sets.value("on_queue_font").value<QFont>());
    ui->onQueueFontColor->setColor(sets.value("on_queue_font_color").value<QColor>());
    ui->onErrorColor->setColor(sets.value("on_error_color").value<QColor>());
    ui->onErrorFont->setFont(sets.value("on_error_font").value<QFont>());
    ui->onErrorFontColor->setColor(sets.value("on_error_font_color").value<QColor>());
    ui->onFinishColor->setColor(sets.value("on_finish_color").value<QColor>());
    ui->onFinishFont->setFont(sets.value("on_finish_font").value<QFont>());
    ui->onFinishFontColor->setColor(sets.value("on_finish_font_color").value<QColor>());
    ui->tableWordWrap->setChecked(sets.value("table_word_wrap").toBool());

    ui->logDir->setText(sets.value("log_dir").toString());
    ui->logAutisave->setChecked(sets.value("log_autosave").toBool());
    ui->logLifeTime->setValue(sets.value("log_life_time").toInt());

    ui->infoColor->setColor(sets.value("log_info_color").value<QColor>());
    ui->infoFont->setFont(sets.value("log_info_font").value<QFont>());
    ui->infoFontColor->setColor(sets.value("log_info_font_color").value<QColor>());
    ui->warningColor->setColor(sets.value("log_warning_color").value<QColor>());
    ui->warningFont->setFont(sets.value("log_warning_font").value<QFont>());
    ui->warningFontColor->setColor(sets.value("log_warning_font_color").value<QColor>());
    ui->errorColor->setColor(sets.value("log_error_color").value<QColor>());
    ui->errorFont->setFont(sets.value("log_error_font").value<QFont>());
    ui->errorFontColor->setColor(sets.value("log_error_font_color").value<QColor>());
    ui->outColor->setColor(sets.value("log_out_color").value<QColor>());
    ui->outFont->setFont(sets.value("log_out_font").value<QFont>());
    ui->outFontColor->setColor(sets.value("log_out_font_color").value<QColor>());
    ui->inColor->setColor(sets.value("log_in_color").value<QColor>());
    ui->inFont->setFont(sets.value("log_in_font").value<QFont>());
    ui->inFontColor->setColor(sets.value("log_in_font_color").value<QColor>());

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
    QToolButton *btn = qobject_cast<QToolButton*>(sender());
    if(!btn) return;

    QFileDialog *dlg = new QFileDialog(this);

    if(ui->browseButton == btn)
    {
        dlg->setDirectory(ui->downDir->text());
        connect(dlg,SIGNAL(fileSelected(QString)),this,SLOT(setDownDir(QString)));
    }
    else if(ui->logBrowseButton == btn)
    {
        dlg->setDirectory(ui->logDir->text());
        connect(dlg,SIGNAL(fileSelected(QString)),ui->logDir,SLOT(setText(QString)));
    }
    else
    {
        delete(dlg);
        return;
    }
    dlg->setDirectory(ui->downDir->text());
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setFileMode(QFileDialog::DirectoryOnly);
    dlg->setWindowTitle(tr("Выбор директории"));
    dlg->setOption(QFileDialog::DontUseNativeDialog);
    dlg->setModal(true);

    dlg->show();
}

void SettingsDialog::setSettingAttribute(const QString &key, const QVariant &value)
{
    sets.insert(key,value);
    cancelSets();
}

void SettingsDialog::updateInterface()
{
    cancelSets();
    emit newSettings();
}

void SettingsDialog::resetFontsColors()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    bool all = false;
    if(!btn) all = true;

    if(all || btn == ui->fontsColorsReset)
    {
        ui->onPauseColor->setColor(QColor("#fff3a4"));
        ui->onPauseFont->setFont(QApplication::font());
        ui->onPauseFontColor->setColor(QColor("#111111"));

        ui->onQueueColor->setColor(QColor("#ffbdbd"));
        ui->onQueueFont->setFont(QApplication::font());
        ui->onQueueFontColor->setColor(QColor("#111111"));

        ui->onLoadColor->setColor(QColor("#b4e1b4"));
        ui->onLoadFont->setFont(QApplication::font());
        ui->onLoadFontColor->setColor(QColor("#111111"));

        ui->onErrorColor->setColor(QColor("#ff5757"));
        ui->onErrorFont->setFont(QApplication::font());
        ui->onErrorFontColor->setColor(QColor("#111111"));

        ui->onFinishColor->setColor(QColor("#abc2c8"));
        ui->onFinishFont->setFont(QApplication::font());
        ui->onFinishFontColor->setColor(QColor("#111111"));
    }

    if(all || btn == ui->logFontColorReset)
    {
        ui->infoColor->setColor(QColor("#dbe8f1"));
        ui->infoFont->setFont(QApplication::font());
        ui->infoFontColor->setColor(QColor("#111111"));

        ui->warningColor->setColor(QColor("#f8ffaf"));
        ui->warningFont->setFont(QApplication::font());
        ui->warningFontColor->setColor(QColor("#111111"));

        ui->errorColor->setColor(QColor("#ffd6d6"));
        ui->errorFont->setFont(QApplication::font());
        ui->errorFontColor->setColor(QColor("#111111"));

        ui->outColor->setColor(QColor("#c8ffc8"));
        ui->outFont->setFont(QApplication::font());
        ui->outFontColor->setColor(QColor("#111111"));

        ui->inColor->setColor(QColor("#e5eefd"));
        ui->inFont->setFont(QApplication::font());
        ui->inFontColor->setColor(QColor("#111111"));
    }
}
