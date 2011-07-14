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
    explicit AddTaskDialog(const QString &dir, QWidget *parent = 0);
    explicit AddTaskDialog(const QString &dir, QSqlDatabase &db_, QWidget *parent = 0);
    void setValidProtocols(const QHash<QString,int> &schemes);

    ~AddTaskDialog();

signals:
    void addedNewTask();

protected:
    void changeEvent(QEvent *e);
    void construct();
    void loadDatabaseData();
    void scanClipboard();

protected slots:
    void updateLocation(int index);
    void urlValidator();

private:
    Ui::AddTaskDialog *ui;

    QSqlDatabase *mydb;
    QString downDir;
    QMap<int, QString> dirs;
    QList<QString> protocols;
};

#endif // ADDTASKDIALOG_H
