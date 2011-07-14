#include "addtaskdialog.h"
#include "ui_addtaskdialog.h"

AddTaskDialog::AddTaskDialog(const QString &dir, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTaskDialog)
{
    ui->setupUi(this);

    mydb = &QSqlDatabase::database();

    ui->locationEdit->setText(dir);
    downDir = dir;

    construct();
}

AddTaskDialog::AddTaskDialog(const QString &dir, QSqlDatabase &db_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddTaskDialog)
{
    ui->setupUi(this);

    mydb = &db_;

    ui->locationEdit->setText(dir);
    downDir = dir;

    construct();
}

void AddTaskDialog::construct()
{
    setWindowTitle("REXLoader - "+tr("New task"));

    loadDatabaseData();
    connect(ui->categoryBox,SIGNAL(currentIndexChanged(int)),this,SLOT(updateLocation(int)));

    setAttribute(Qt::WA_AlwaysShowToolTips);
    scanClipboard();
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
    qr.exec("SELECT * FROM categories");
    int otherId = 0;
    qDebug()<<downDir;
    while(qr.next())
    {
        QString cattitle;
        if(qr.value(1).toString() == "#downloads")continue;
        else if(qr.value(1).toString() == "#archives"){cattitle = tr("Archives"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#apps"){cattitle = tr("Applications"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#audio"){cattitle = tr("Audio"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#video"){cattitle = tr("Video"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#other"){cattitle = tr("All Downloads"); otherId = qr.value(0).toInt();dirs.insert(qr.value(0).toInt(),downDir);}
        else {cattitle = qr.value(1).toString(); dirs.insert(qr.value(0).toInt(),qr.value(2).toString());}
        qDebug()<<qr.value(1).toString();
        ui->categoryBox->addItem(cattitle, qr.value(0).toInt());
    }
    ui->categoryBox->setCurrentIndex(ui->categoryBox->findData(QVariant(otherId)));

}

void AddTaskDialog::updateLocation(int index)
{
    int catId = ui->categoryBox->itemData(index).toInt();
    ui->locationEdit->setText(dirs.value(catId));
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
