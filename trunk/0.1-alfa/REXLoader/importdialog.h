/*
Project: REXLoader (Downloader), Source file: importdialog.h
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

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <QDialog>
#include <QtSql/QtSql>

namespace Ui {
    class ImportDialog;
}

class ImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDialog(const QString &dir, QWidget *parent = 0);
    explicit ImportDialog(const QString &dir, QSqlDatabase &db_, QWidget *parent = 0);
    ~ImportDialog();

protected:
    void loadDatabaseData();

private:
    void initialize();

    Ui::ImportDialog *ui;
    QSqlDatabase mydb;

    QString downDir;
    QMap<int, QString> dirs;
};

#endif // IMPORTDIALOG_H
