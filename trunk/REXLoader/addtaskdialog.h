#ifndef ADDTASKDIALOG_H
#define ADDTASKDIALOG_H

#include <QDialog>
#include <QtSql/QtSql>
#include <QClipboard>
#include <QMimeData>
#include <QFileDialog>
#include <QMessageBox>

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

private:
    Ui::AddTaskDialog *gui;

    QSqlDatabase mydb;
    QString downDir;
    QMap<int, QString> dirs;
    QList<QString> protocols;
    int priority;
};

#endif // ADDTASKDIALOG_H
