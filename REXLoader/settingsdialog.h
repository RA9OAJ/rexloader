/*
Project: REXLoader (Downloader), Source file: settingsdialog.h
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

#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QHash>
#include <QDesktopWidget>
#include <QTimer>
#include <QFileDialog>

namespace Ui {
    class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
    QList<QString> keys() const;
    QVariant value(const QString &key) const;

signals:
    void newSettings();

public slots:
    void show();
    void setSettingAttribute(const QString &key, const QVariant &value);
    void updateInterface();

protected:
    void closeEvent(QCloseEvent *event);

protected slots:
    void selectSubSettings();
    void applySets();
    void cancelSets();
    void applyAndClose();
    void setDownDir(const QString &dir);
    void showFileDialog();

private:
    Ui::SettingsDialog *ui;
    int last_row;

    QHash<QString,QVariant> sets;
};

#endif // SETTINGSDIALOG_H
