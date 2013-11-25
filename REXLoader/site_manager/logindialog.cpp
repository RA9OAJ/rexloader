#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    def_btn = 0;
    tmr.setInterval(1000);
    window_title = windowTitle();

    connect(&tmr,SIGNAL(timeout()),this,SLOT(timeoutReaction()));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::setWindowTitle(const QString &title)
{
    window_title = title;
    QDialog::setWindowTitle(title);
}

void LoginDialog::setText(const QString &text)
{
    ui->textLabel->setText(text);
}

void LoginDialog::getAuth(const QUrl &url)
{
    _url = url;
    setText(tr("To access an object at <a href='%1'>%1</a> requires authorization.<br/>Please enter your username and password.").
            arg(url.toString()));
    show();
}

void LoginDialog::setDefaultButton(QDialogButtonBox::StandardButton std_btn, int timeout)
{
    def_btn = ui->buttonBox->button(std_btn);
    if(def_btn)
        def_btn_txt = def_btn->text();

    if(timeout > -1)
        def_timeout = timeout;

}

void LoginDialog::setDefaultButtonTimeout(int sec)
{
    if(sec > -1)
        def_timeout = sec;
}

void LoginDialog::accept()
{
    emit authData(_url, QString("%1\r\n%2").arg(ui->loginEdit->text(),ui->passwordEdit->text()));
    QDialog::accept();

    if(def_btn)
        tmr.start();
}

void LoginDialog::show()
{
    tmr.start();
    QDialog::show();
}


void LoginDialog::timeoutReaction()
{
    if(!def_btn)
        return;

    --def_timeout;

    def_btn->setText(def_btn_txt + QString(" (%1)").arg(QString::number(def_timeout)));
    QDialog::setWindowTitle(window_title + QString(" (%1)").arg(QString::number(def_timeout)));

    if(!def_timeout)
    {
        tmr.stop();
        def_btn->click();
    }
}
