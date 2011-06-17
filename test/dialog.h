#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include <QMessageBox>
#include <QPluginLoader>
#include <QTranslator>
#include "../Httploader/LoaderInterface.h"
#include <QDebug>
#include <QUrl>

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

public slots:
    void startDownload();
    void stopDownload();
    void showDirBrowser();
    void scanTaskStatus();
    void _test();
    void setSpeed(int _spd);

private:
    Ui::Dialog *ui;
    LoaderInterface* loader;
    int task_id;
    qint64 total;
    qint64 loaded;
    qint64 last_spd;
    QString dir_;
    int pool[9];
};

#endif // DIALOG_H
