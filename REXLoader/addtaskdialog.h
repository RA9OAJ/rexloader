#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>
#include <QtSql/QtSql>

namespace Ui {
    class AddTaskDialog;
}

class AddTaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTaskDialog(QWidget *parent = 0);
    explicit AddTaskDialog(QSqlDatabase &db_, QWidget *parent = 0);
    ~AddTaskDialog();

signals:
    void addedNewTask();

protected:
    void changeEvent(QEvent *e);
    void loadDatabaseData();

private:
    Ui::AddTaskDialog *ui;

    QSqlDatabase *mydb;
};

#endif // ADDTASKDIALOG_H
