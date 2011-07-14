#include "addtaskdialog.h"
#include "ui_addtaskdialog.h"

AddTaskDialog::AddTaskDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTaskDialog)
{
    ui->setupUi(this);

    mydb = &QSqlDatabase::database();
    loadDatabaseData();
    setAttribute(Qt::WA_AlwaysShowToolTips);
    scanClipboard();
}

AddTaskDialog::AddTaskDialog(QSqlDatabase &db_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTaskDialog)
{
    ui->setupUi(this);

    mydb = &db_;
    loadDatabaseData();
    scanClipboard();
}

AddTaskDialog::~AddTaskDialog()
{
    delete ui;
}

void AddTaskDialog::loadDatabaseData()
{
    QSqlQuery qr(*mydb);
    qr.prepare("SELECT id,url FROM tasks ORDER BY id ASC LIMIT 5");
    qr.exec();

    while(qr.next())
        ui->urlBox->addItem(qr.value(1).toString());

    ui->urlBox->setCurrentIndex(-1);
}

void AddTaskDialog::scanClipboard()
{
    const QClipboard *clipbrd = QApplication::clipboard();

    QUrl url(clipbrd->text());
    if(url.isValid() && url.scheme() != "")
        ui->urlBox->setEditText(clipbrd->text());
}

void AddTaskDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
