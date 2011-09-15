/*
Project: REXLoader (Downloader), Source file: AddTaskDialog.h
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

#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>
#include <QtSql/QtSql>
#include <QClipboard>
#include <QMimeData>
#include <QFileDialog>
#include <QDesktopWidget>
#include "emessagebox.h"

namespace Ui {
    class AddTaskDialog;
}

class AddTaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTaskDialog(const QString &dir, QWidget *parent = 0);
    explicit AddTaskDialog(const QString &dir, QSqlDatabase &db_, QWidget *parent = 0);
    void setValidProtocols(const QHash<QString,int> &schemes);
    void setNewUrl(const QString &url);
    void setAdditionalInfo(const QString &flnm, qint64 cursz, qint64 totalsz, const QString &mime);

    ~AddTaskDialog();

signals:
    void addedNewTask();

protected:
    void changeEvent(QEvent *e);
    void construct();
    void loadDatabaseData();
    void scanClipboard();
    void addTask();

protected slots:
    void updateLocation(int index);
    void urlValidator();
    void openDirDialog();
    void startNow();
    void startLater();
    void acceptQAction(QAbstractButton *btn); //обработчик решений пользователя на задаваемые программой вопросы

private:
    Ui::AddTaskDialog *gui;

    QSqlDatabase mydb;
    QString downDir;
    QMap<int, QString> dirs;
    QList<QString> protocols;
    int priority;

    static int obj_cnt;
    QString myfilename;
    qint64 currentsize;
    qint64 totalsize;
    QString mymime;
    bool additional_flag;
};

#endif // ADDTASKDIALOG_H
