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

void AddTaskDialog::setDefaultDir(const QString &dir)
{
    ui->locationEdit->setText(dir);
}

AddTaskDialog::~AddTaskDialog()
{
    delete ui;
}

void AddTaskDialog::loadDatabaseData()
{
    QSqlQuery qr(*mydb);
    qr.exec("SELECT id,url FROM tasks ORDER BY id ASC LIMIT 5");

    while(qr.next())
        ui->urlBox->addItem(qr.value(1).toString());
    ui->urlBox->setCurrentIndex(-1);

    qr.clear();
    qr.exec("SELECT * FROM categories ORDER BY parent ASC");
    int otherId = 0;
    while(qr.next())
    {
        QString cattitle;
        if(qr.value(1).toString() == "#downloads")continue;
        else if(qr.value(1).toString() == "#archives")cattitle = tr("Archives");
        else if(qr.value(1).toString() == "#apps")cattitle = tr("Applications");
        else if(qr.value(1).toString() == "#audio")cattitle = tr("Audio files");
        else if(qr.value(1).toString() == "#video")cattitle = tr("Video files");
        else if(qr.value(1).toString() == "#other"){cattitle = tr("Other"); otherId = qr.value(0).toInt();}

        ui->categoryBox->addItem(cattitle, qr.value(0).toInt());
    }
    ui->categoryBox->setCurrentIndex(ui->categoryBox->findData(QVariant(otherId)));

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
