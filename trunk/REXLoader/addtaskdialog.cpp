#include "addtaskdialog.h"
#include "ui_addtaskdialog.h"

AddTaskDialog::AddTaskDialog(const QString &dir, QWidget *parent) :
    QDialog(parent),
    gui(new Ui::AddTaskDialog)
{
    gui->setupUi(this);

    mydb = &QSqlDatabase::database();

    gui->locationEdit->setText(dir);
    downDir = dir;

    construct();
}

AddTaskDialog::AddTaskDialog(const QString &dir, QSqlDatabase &db_, QWidget *parent) :
    QDialog(parent),
    gui(new Ui::AddTaskDialog)
{
    gui->setupUi(this);

    mydb = &db_;

    gui->locationEdit->setText(dir);
    downDir = dir;

    construct();
}

void AddTaskDialog::construct()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("REXLoader - "+tr("New task"));

    loadDatabaseData();
    connect(gui->categoryBox,SIGNAL(currentIndexChanged(int)),this,SLOT(updateLocation(int)));
    connect(gui->urlBox,SIGNAL(editTextChanged(QString)),this,SLOT(urlValidator()));
    connect(gui->browseButton,SIGNAL(released()),this,SLOT(openDirDialog()));

    setAttribute(Qt::WA_AlwaysShowToolTips);
    gui->errorFrame->setHidden(true);

    scanClipboard();
}

void AddTaskDialog::setNewUrl(const QString &url)
{
    gui->urlBox->setEditText(url);
    urlValidator();
}

void AddTaskDialog::setValidProtocols(const QHash<QString, int> &schemes)
{
    protocols = schemes.keys();
    scanClipboard();
}

void AddTaskDialog::openDirDialog()
{
    QString dir = downDir;

    if(!QDir().exists(downDir))
        QDir().mkpath(downDir);

    if(QDir().exists(gui->locationEdit->text()))dir = gui->locationEdit->text();

    dir = QFileDialog::getExistingDirectory(this,tr("Folder select"),dir);

    if(!dir.isEmpty())gui->locationEdit->setText(dir);
}

void AddTaskDialog::urlValidator()
{
    if(protocols.isEmpty())return;

    QUrl url(gui->urlBox->currentText());
    QString errorStr;

    if(!url.isValid())errorStr = tr("URL is not correct. Enter another URL or adjust it.");
    else if(!protocols.contains(url.scheme().toLower()))errorStr = tr("This protocol is not supported. Check whether there is a plugin to work with the protocol and whether it is enabled.");

    if(errorStr.isEmpty())
    {
        gui->errorFrame->setHidden(true);
        gui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(false);
    }
    else
    {
        gui->errorLabel->setText(errorStr);
        gui->errorFrame->setHidden(false);
        gui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
    }
}

AddTaskDialog::~AddTaskDialog()
{
    delete gui;
}

void AddTaskDialog::loadDatabaseData()
{
    QSqlQuery qr(*mydb);
    qr.exec("SELECT id,url FROM tasks ORDER BY id ASC LIMIT 5");

    while(qr.next())
        gui->urlBox->addItem(qr.value(1).toString());
    gui->urlBox->setCurrentIndex(-1);

    qr.clear();
    qr.exec("SELECT * FROM categories");
    int otherId = 0;

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

        gui->categoryBox->addItem(cattitle, qr.value(0).toInt());
    }
    gui->categoryBox->setCurrentIndex(gui->categoryBox->findData(QVariant(otherId)));

}

void AddTaskDialog::updateLocation(int index)
{
    int catId = gui->categoryBox->itemData(index).toInt();
    if(!dirs.contains(catId))dirs.insert(catId,downDir);
    gui->locationEdit->setText(dirs.value(catId));
}

void AddTaskDialog::scanClipboard()
{
    const QClipboard *clipbrd = QApplication::clipboard();

    QUrl url(clipbrd->text());

    if(url.isValid() && protocols.contains(url.scheme().toLower()))
        gui->urlBox->setEditText(clipbrd->text());
}

void AddTaskDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        gui->retranslateUi(this);
        break;
    default:
        break;
    }
}
