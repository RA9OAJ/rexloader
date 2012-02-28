/*
Project: REXLoader (Downloader), Source file: taskdialog.h
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

#ifndef TASKDIALOG_H
#define TASKDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QDesktopWidget>
#include "titemmodel.h"

namespace Ui {
    class TaskDialog;
}

class TaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TaskDialog(QWidget *parent = 0);
    ~TaskDialog();

    void setSourceData(TItemModel *model, QModelIndex index, const QHash<int,LoaderInterface*> &pluglist, const QHash<int,int> &list);

public slots:
    void deleteThisTask(QModelIndex index);

protected slots:
    void scheduler();
    void pressAnaliser();
    void moveToCenter();

signals:
    void startTask(int id);
    void stopTask(int id);
    void redownloadTask(int id);

private:
    Ui::TaskDialog *ui;
    const QHash<int,int> *lst;
    QPointer<TItemModel> mdl;
    QModelIndex idx;
    const QHash<int,LoaderInterface*> *ldr;

    static int obj_cnt;
};

#endif // TASKDIALOG_H
