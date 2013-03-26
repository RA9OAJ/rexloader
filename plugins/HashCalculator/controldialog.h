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
    void setFileNames(const QStringList &file_name);
public slots:
    void progress(QString file_name, int percent);
    void calcFinished(QString file_name, QString md5_result, QString sha1_result);
    int exec();
private:
    Ui::ControlDialog *ui;
    HashCalculatorThread *mp_hash_calc_thread;
    int row;

private slots:
    void slotClose();
};

#endif // CONTROLDIALOG_H
