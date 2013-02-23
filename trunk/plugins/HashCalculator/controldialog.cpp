#include "controldialog.h"
#include "ui_controldialog.h"


ControlDialog::ControlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ControlDialog)
{
    ui->setupUi(this);
    mp_hash_calc_thread = new HashCalculatorThread(this);
    connect(mp_hash_calc_thread, SIGNAL(progress(int)), this, SLOT(percent(int)), Qt::QueuedConnection);
    connect(mp_hash_calc_thread, SIGNAL(s_md5(QByteArray)), this, SLOT(md5_result(QByteArray)), Qt::QueuedConnection);
    connect(mp_hash_calc_thread, SIGNAL(s_sha1(QByteArray)), this, SLOT(sha1_result(QByteArray)), Qt::QueuedConnection);
    connect(ui->btnStop, SIGNAL(clicked()), this, SLOT(slotClose()));
}


ControlDialog::~ControlDialog()
{
    delete ui;
}


void ControlDialog::setFileName(const QString &file_name)
{
    mp_hash_calc_thread->setFileName(file_name);
}


void ControlDialog::percent(int percent)
{
    ui->progressBar->setValue(percent);
}


int ControlDialog::exec()
{
    mp_hash_calc_thread->start();
    return QDialog::exec();
}


void ControlDialog::md5_result(QByteArray result)
{
    ui->md5_result->setText(result);
}


void ControlDialog::sha1_result(QByteArray result)
{
    ui->sha1_result->setText(result);
}


void ControlDialog::slotClose()
{
    mp_hash_calc_thread->stop();
    ui->md5_result->setText(QString(""));
    ui->sha1_result->setText(QString(""));
    reject();
}
