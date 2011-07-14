#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>
#include <QtSql/QtSql>
#include <QClipboard>
#include <QMimeData>

namespace Ui {
    class AddTaskDialog;
}

class AddTaskDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTaskDialog(QWidget *parent = 0);
    explicit AddTaskDialog(QSqlDatabase &db_, QWidget *parent = 0);

    void setDefaultDir(const QString &dir);

    ~AddTaskDialog();

signals:
    void addedNewTask();

protected:
    void changeEvent(QEvent *e);
    void loadDatabaseData();
    void scanClipboard();

private:
    Ui::AddTaskDialog *ui;

    QSqlDatabase *mydb;
};

#endif // ADDTASKDIALOG_H
