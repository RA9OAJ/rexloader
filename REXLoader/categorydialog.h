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

#ifndef CATEGORYDIALOG_H
#define CATEGORYDIALOG_H

#include <QDialog>
#include <QtGui>
#include "treeitemmodel.h"
#include "filenamevalidator.h"

namespace Ui {
    class CategoryDialog;
}

class CategoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CategoryDialog(QWidget *parent = 0);
    explicit CategoryDialog(QSqlDatabase &db_, QWidget *parent = 0);
    ~CategoryDialog();

    void setParentCategory(int parent);
    void setCategory(int id, int parent);
    void setCategoryTitle(const QString &title);
    void setCategoryExtList(const QString &extlist);

public slots:
    void setCategoryDir(const QString &dir);

signals:
    void canUpdateModel(QString cat_title, int row, int parent_id, int cat_id);

protected slots:
    void applyCategory();
    void showDirDialog();
    void formValidator();

private:
    void initialize();
    Ui::CategoryDialog *ui;
    TreeItemModel *model;

    QSqlDatabase mydb;
    int internal_id;
    int myrow;
    int myparent_id;
};

#endif // CATEGORYDIALOG_H
