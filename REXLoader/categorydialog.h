#ifndef CATEGORYDIALOG_H
#define CATEGORYDIALOG_H

#include <QDialog>
#include <QtGui>
#include "treeitemmodel.h"

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

signals:
    void canUpdateModel(QString cat_title, int row, int parent_id);

protected slots:
    void setCategoryDir(const QString &dir);
    void applyCategory();
    void showDirDialog();
    void formValidator();

private:
    void initialize();
    Ui::CategoryDialog *ui;
    TreeItemModel *model;

    QSqlDatabase mydb;
    int internal_id;
};

#endif // CATEGORYDIALOG_H
