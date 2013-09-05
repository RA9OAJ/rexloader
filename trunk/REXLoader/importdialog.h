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

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include <QtSql/QtSql>
#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidgetSelectionRange>

#include "importmaster.h"

namespace Ui {
    class ImportDialog;
}

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(const QStringList &files, QWidget *parent = 0);
    explicit ImportDialog(const QStringList &files, QSqlDatabase &db_, QWidget *parent = 0);
    ~ImportDialog();

    void setDownDir(const QString &dir);
    void addProtocol(const QString &proto);

signals:
    void addedNewTask();

public slots:
    void import();
    void addUrl(const QString &url);

protected:
    void loadDatabaseData();
    int getCategory(const QString &file);
    bool contains(const QString &url) const;

protected slots:
    void selectAll();
    void deselectAll();
    void invertSelection();
    void locationSelected(int id);
    void selectionChanged();
    void setCategory(int id);
    void setDownloadLater();
    void showDirDialog();
    void addTasks();
    bool addTask(QTableWidgetItem *itm);
    void showWarning() const;

private:
    void initialize();

    Ui::ImportDialog *ui;
    QSqlDatabase mydb;

    QString downDir;
    QMap<int, QString> dirs;
    QStringList imp_files;
    qint64 fndurl;
    QString tasks_exists;
    QStringList protocols;
};

#endif // IMPORTDIALOG_H
