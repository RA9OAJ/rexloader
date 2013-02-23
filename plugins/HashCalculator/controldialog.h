#ifndef CONTROLDIALOG_H
#define CONTROLDIALOG_H

#include <QDialog>
#include "HashCalculatorThread.h"

namespace Ui {
class ControlDialog;
}

class ControlDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ControlDialog(QWidget *parent = 0);
    ~ControlDialog();
    void setFileName(const QString &file_name);
public slots:
    void percent(int percent);
    int exec();
private:
    Ui::ControlDialog *ui;
    HashCalculatorThread *mp_hash_calc_thread;

public slots:
    //void start();

private slots:
    void md5_result(QByteArray result);
    void sha1_result(QByteArray result);
    void slotClose();
};

#endif // CONTROLDIALOG_H
