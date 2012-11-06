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
#include "pluginlistmodel.h"
#include "pluginitemdelegate.h"

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
    void setPlugListModel(PluginListModel* model);

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
    void resetFontsColors();
    void updatePluginListBox(const QModelIndex &index);
    void updatePluginStatus(int index);
    void disableLogUserFonColorStyle(bool flag = true);

private:
    Ui::SettingsDialog *ui;
    int last_row;
    PluginListModel *mdl;
    PluginItemDelegate *delegate;

    QHash<QString, int> plugproto;
    QHash<QString,QVariant> sets;
};

#endif // SETTINGSDIALOG_H
