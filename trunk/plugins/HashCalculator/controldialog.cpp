#include "controldialog.h"
#include "ui_controldialog.h"

ControlDialog::ControlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ControlDialog)
{
    ui->setupUi(this);
}

ControlDialog::~ControlDialog()
{
    delete ui;
}
