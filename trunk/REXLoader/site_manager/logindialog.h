#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>
#include <QUrl>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();
    void setWindowTitle(const QString &title);

public slots:
    void setText(const QString &text);
    void getAuth(const QUrl &url);
    void setDefaultButton(QDialogButtonBox::StandardButton std_btn, int timeout = -1);
    void setDefaultButtonTimeout(int sec);
    void accept();
    void show();
    
signals:
    void authData(const QUrl &url, const QString &auth);

protected slots:
    void timeoutReaction();

private:
    Ui::LoginDialog *ui;
    QUrl _url;
    QTimer tmr;
    QPushButton *def_btn;
    int def_timeout;
    QString def_btn_txt;
    QString window_title;
};

#endif // LOGINDIALOG_H
