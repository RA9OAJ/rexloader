/*
Project: REXLoader (Downloader), Source file: AddTaskDialog.cpp
Copyright (C) 2011  Sarvaritdinov R.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "addtaskdialog.h"
#include "ui_addtaskdialog.h"

AddTaskDialog::AddTaskDialog(const QString &dir, QWidget *parent) :
    QDialog(parent),
    gui(new Ui::AddTaskDialog)
{
    gui->setupUi(this);

    mydb = QSqlDatabase::database();

    gui->locationEdit->setText(dir);
    downDir = dir;

    construct();
}

AddTaskDialog::AddTaskDialog(const QString &dir, QSqlDatabase &db_, QWidget *parent) :
    QDialog(parent),
    gui(new Ui::AddTaskDialog)
{
    gui->setupUi(this);

    mydb = db_;

    gui->locationEdit->setText(dir);
    downDir = dir;

    construct();
}

void AddTaskDialog::construct()
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("REXLoader - "+tr("New task"));
    priority = 2; //нормальный приоритет

    loadDatabaseData();
    connect(gui->categoryBox,SIGNAL(currentIndexChanged(int)),this,SLOT(updateLocation(int)));
    connect(gui->urlBox,SIGNAL(editTextChanged(QString)),this,SLOT(urlValidator()));
    connect(gui->browseButton,SIGNAL(released()),this,SLOT(openDirDialog()));
    connect(gui->startNowBtn,SIGNAL(released()),this,SLOT(startNow()));
    connect(gui->startLaterBtn,SIGNAL(released()),this,SLOT(startLater()));

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
        gui->startNowBtn->setEnabled(true);
        gui->startLaterBtn->setEnabled(true);
    }
    else
    {
        gui->errorLabel->setText(errorStr);
        gui->errorFrame->setHidden(false);
        gui->startNowBtn->setEnabled(false);
        gui->startLaterBtn->setEnabled(false);
    }
}

AddTaskDialog::~AddTaskDialog()
{
    delete gui;
}

void AddTaskDialog::loadDatabaseData()
{
    QSqlQuery qr(mydb);
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

void AddTaskDialog::startNow()
{
    priority = 2;
    addTask();
}

void AddTaskDialog::startLater()
{
    priority = -1;
    addTask();
}

void AddTaskDialog::addTask()
{
    QSqlQuery qr(mydb);

    qr.prepare("SELECT count(*) FROM tasks WHERE url=:url;");
    qr.bindValue("url", gui->urlBox->currentText());
    if(!qr.exec())
    {
        //тут запись в журнал ошибок
        qDebug()<<"void AddTaskDialog::addTask(1):"<<qr.executedQuery()<<" Error: "<<qr.lastError().text();
        close();
        return;
    }
    qr.next();

    if(qr.value(0).toInt() > 0)
    {
        EMessageBox question(this);
        question.setIcon(QMessageBox::Question);
        question.addButton(tr("Redownload"), QMessageBox::YesRole);
        question.addButton(QMessageBox::Cancel);
        question.setDefaultButton(QMessageBox::Cancel);
        question.setText(tr("This URL is already in jobs."));
        question.setInformativeText(tr("Click <b>\"Redownload\"</b> to add a task or <b>\"Cancel\"</b> to cancel this action."));

        int ans = question.exec();
        if(ans == QMessageBox::Cancel)
        {
            close();
            return;
        }
        qr.clear();
        qr.prepare("DELETE FROM tasks WHERE url=:url;");
        qr.bindValue("url",gui->urlBox->currentText());

        if(!qr.exec())
        {
            //тут запись в журнал ошибок
            qDebug()<<"void AddTaskDialog::addTask(2): Error: "<<qr.lastError().text();
            close();
            return;
        }
    }

    int catId = 0;
    QVariant udata = gui->categoryBox->itemData(gui->categoryBox->currentIndex()).toInt();

    if(!udata.toInt() || gui->categoryBox->findText(gui->categoryBox->currentText()) < 0)
    {
        qr.clear();
        qr.prepare("INSERT INTO categories(title,dir,extlist) VALUES(:title,:dir,'');");
        qr.bindValue("title",gui->categoryBox->currentText());
        qr.bindValue("dir",gui->locationEdit->text());

        if(!qr.exec())
        {
            //тут запись в журнал ошибок
            qDebug()<<"void AddTaskDialog::addTask(3): Error: "<<qr.lastError().text();
            close();
            return;
        }

        qr.clear();
        qr.prepare("SELECT id FROM categories WHERE title=:title AND dir=:dir;");
        qr.bindValue("title",gui->categoryBox->currentText());
        qr.bindValue("dir",gui->locationEdit->text());

        if(!qr.exec())
        {
            //тут запись в журнал ошибок
            qDebug()<<"void AddTaskDialog::addTask(4): Error: "<<qr.lastError().text();
            close();
            return;
        }
        qr.next();
        catId = qr.value(0).toInt();
    }
    else catId = gui->categoryBox->itemData(gui->categoryBox->currentIndex()).toInt();

    QDir().mkpath(gui->locationEdit->text());
    QDateTime dtime(QDateTime::currentDateTime());
    QFileInfo flinfo(gui->urlBox->currentText());
    QString flname = (flinfo.fileName() != QString() ? flinfo.fileName() : "noname.html");
    flname = gui->locationEdit->text() + "/" + flname + "." + dtime.toString("yyyyMMddhhmmss") + ".rldr";

    qr.clear();
    qr.prepare("INSERT INTO tasks(url,filename,datecreate,tstatus,categoryid,priority,note) VALUES(:url,:filename,:datecreate,:tstatus,:categoryid,:priority,:note);");
    qr.bindValue("url", gui->urlBox->currentText());
    qr.bindValue("filename",flname);
    qr.bindValue("datecreate",dtime.toString("yyyy-MM-ddThh:mm:ss"));
    if(priority < 0)qr.bindValue("tstatus",0);
    else qr.bindValue("tstatus",-100);
    qr.bindValue("categoryid",catId);
    qr.bindValue("priority",2);
    qr.bindValue("note",gui->textEdit->document()->toPlainText());

    if(!qr.exec())
    {
        //тут запись в журнал ошибок
        qDebug()<<"void AddTaskDialog::addTask(5): Error: "<<qr.lastError().text();
        close();
        return;
    }

    emit addedNewTask();
    close();
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
