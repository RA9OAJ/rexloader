#ifndef CATEGORYDIALOG_H
#define CATEGORYDIALOG_H

#include <QDialog>

namespace Ui {
    class CategoryDialog;
}

class CategoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CategoryDialog(QWidget *parent = 0);
    ~CategoryDialog();

private:
    Ui::CategoryDialog *ui;
};

#endif // CATEGORYDIALOG_H
