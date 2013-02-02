#ifndef CONTROLDIALOG_H
#define CONTROLDIALOG_H

#include <QDialog>

namespace Ui {
class ControlDialog;
}

class ControlDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ControlDialog(QWidget *parent = 0);
    ~ControlDialog();
    
private:
    Ui::ControlDialog *ui;
};

#endif // CONTROLDIALOG_H
