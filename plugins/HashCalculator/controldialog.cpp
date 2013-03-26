#include "controldialog.h"
#include "ui_controldialog.h"


ControlDialog::ControlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ControlDialog)
{
    ui->setupUi(this);
    mp_hash_calc_thread = new HashCalculatorThread(this);
    QStringList headers;
    headers << tr("Имя файла") << "md5" << "sha1";
    row = 0;
    ui->table->setHorizontalHeaderLabels(headers);
    ui->table->setColumnCount(3);
    connect(mp_hash_calc_thread, SIGNAL(progress(QString, int)),
            this, SLOT(progress(QString,int)), Qt::QueuedConnection);
    connect(mp_hash_calc_thread, SIGNAL(calcFinished(QString,QString,QString)),
            this, SLOT(calcFinished(QString,QString,QString)), Qt::QueuedConnection);
}

ControlDialog::~ControlDialog()
{
    delete ui;
}

void ControlDialog::setFileNames(const QStringList &file_name)
{
    mp_hash_calc_thread->setFileNames(file_name);
    row = 0;
    ui->table->clearContents();
    ui->table->setRowCount(file_name.size());
}

void ControlDialog::progress(QString file_name, int percent)
{
    Q_UNUSED(file_name)
    ui->progressBar->setValue(percent);
    ui->file_name->setText(file_name);
}

void ControlDialog::calcFinished(QString file_name, QString md5_result, QString sha1_result)
{
    QTableWidgetItem *name = new QTableWidgetItem(file_name);
    QTableWidgetItem *md5  = new QTableWidgetItem(md5_result);
    QTableWidgetItem *sha1 = new QTableWidgetItem(sha1_result);
    ui->table->setItem(row, 0, name);
    ui->table->setItem(row, 1, md5);
    ui->table->setItem(row, 2, sha1);
    row++;
    ui->table->resizeColumnsToContents();
//    qDebug() << file_name;
//    qDebug() << md5_result;
//    qDebug() << sha1_result;
}

void ControlDialog::slotClose()
{
    mp_hash_calc_thread->stop();
    reject();
}

int ControlDialog::exec()
{
    mp_hash_calc_thread->start();
    return QDialog::exec();
}
