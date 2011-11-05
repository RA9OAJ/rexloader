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

int AddTaskDialog::obj_cnt = 0; //счетчик объектов класса

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
    ++obj_cnt;
    additional_flag = false;
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("REXLoader - "+tr("Новое задание"));
    priority = 2; //нормальный приоритет
    if(!parent())setWindowIcon(QIcon(":/appimages/trayicon.png"));
    FileNameValidator *validator = new FileNameValidator(this);
    gui->fileName->setValidator(validator);
    gui->categoryBox->setValidator(validator);
    urlValidator();

    loadDatabaseData();
    connect(gui->categoryBox,SIGNAL(currentIndexChanged(int)),this,SLOT(updateLocation(int)));
    connect(gui->urlBox,SIGNAL(editTextChanged(QString)),this,SLOT(urlValidator()));
    connect(gui->browseButton,SIGNAL(released()),this,SLOT(openDirDialog()));
    connect(gui->startNowBtn,SIGNAL(released()),this,SLOT(startNow()));
    connect(gui->startLaterBtn,SIGNAL(released()),this,SLOT(startLater()));
    connect(gui->urlBox,SIGNAL(activated(QString)),this,SLOT(getCategory(QString)));

    QDesktopWidget ds;
    QRect desktop = ds.availableGeometry();
    QPoint top_left = QPoint((desktop.bottomRight().x()-size().width())/2+20*(obj_cnt-1),(desktop.bottomRight().y()-size().height())/2+20*(obj_cnt-1));
    move(top_left);

    setAttribute(Qt::WA_AlwaysShowToolTips);
    gui->errorFrame->setHidden(true);

    scanClipboard();
}

void AddTaskDialog::setNewUrl(const QString &url)
{
    gui->urlBox->addItem(url);
    gui->urlBox->setCurrentIndex(gui->urlBox->findText(url));
    defUrl = url;
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

    dir = QFileDialog::getExistingDirectory(this,tr("Выбор диалога"),dir);

    if(!dir.isEmpty())gui->locationEdit->setText(dir);
}

void AddTaskDialog::urlValidator()
{
    if(gui->urlBox->currentText().isEmpty())
    {
        gui->startNowBtn->setEnabled(false);
        gui->startLaterBtn->setEnabled(false);
        gui->errorFrame->setHidden(true);
        return;
    }

    if(protocols.isEmpty())return;

    QUrl url(gui->urlBox->currentText());
    QString errorStr;

    if(!url.isValid())errorStr = tr("URL не корректен. Введите другой URL или справьте этот.");
    else if(!protocols.contains(url.scheme().toLower()))errorStr = tr("Этот протокол не поддерживается. Проверьте наличие соответствующего плагина и его состояние.");

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
    getCategory(gui->urlBox->currentText());
}

AddTaskDialog::~AddTaskDialog()
{
    delete gui;
    --obj_cnt;
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
        else if(qr.value(1).toString() == "#archives"){cattitle = tr("Архивы"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#apps"){cattitle = tr("Приложения"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#audio"){cattitle = tr("Аудио"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#video"){cattitle = tr("Видео"); dirs.insert(qr.value(0).toInt(),downDir+"/"+cattitle);}
        else if(qr.value(1).toString() == "#other"){cattitle = tr("Другое"); otherId = qr.value(0).toInt();dirs.insert(qr.value(0).toInt(),downDir);}
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

    QUrl url = QUrl::fromEncoded(clipbrd->text().toUtf8());
    if(url.isValid() && protocols.contains(url.scheme().toLower()))
        gui->urlBox->setEditText(url.toString());
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
        EMessageBox *question = new EMessageBox(this);
        question->setIcon(QMessageBox::Question);
        question->setWindowTitle(tr("URL уже существует"));
        question->addButton(tr("Перезакачать"), EMessageBox::YesRole);
        QPushButton *btn = question->addButton(tr("Отмена"),EMessageBox::RejectRole);
        question->setDefaultButton(btn);
        question->setText(tr("Этот URL <a href=\"%1\">%1</a> уже присутствует в списке заданий.").arg(gui->urlBox->currentText()));
        question->setInformativeText(tr("Нажмите <b>\"Перезакачать\"</b> для продолжения или <b>\"Отмена\"</b> для отмены действия."));
        question->setActionType(EMessageBox::AT_REDOWNLOAD);

        connect(question,SIGNAL(buttonClicked(QAbstractButton*)),this,SLOT(acceptQAction(QAbstractButton*)));
        question->show();
        return;
    }

    int catId = 0;
    QVariant udata = gui->categoryBox->itemData(gui->categoryBox->currentIndex()).toInt();

    if(!udata.toInt() || gui->categoryBox->findText(gui->categoryBox->currentText()) < 0)
    {
        qr.clear();
        qr.prepare("INSERT INTO categories(title,dir,extlist,parent_id) VALUES(:title,:dir,'',1);");
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
    QUrl curl = QUrl::fromEncoded(gui->urlBox->currentText().toUtf8());
    QFileInfo flinfo(curl.toString(QUrl::RemoveQuery | QUrl::RemoveFragment));
    QString flname;
    if(gui->fileName->text().isEmpty()) flname = (flinfo.fileName() != QString() ? flinfo.fileName() : "noname.html");
    else flname = gui->fileName->text();
    flname = gui->locationEdit->text() + "/" + flname + "." + dtime.toString("yyyyMMddhhmmss") + ".rldr";

    qr.clear();
    if(additional_flag && gui->urlBox->currentText() == defUrl)
    {
        qr.prepare("INSERT INTO tasks(url,datecreate,filename,currentsize,totalsize,mime,tstatus,categoryid,priority,note) VALUES(:url,:datecreate,:filename,:currentsize,:totalsize,:mime,:tstatus,:categoryid,:priority,:note)");
        qr.bindValue("url", gui->urlBox->currentText());
        qr.bindValue("datecreate",dtime.toString("yyyy-MM-ddThh:mm:ss"));
        qr.bindValue("filename",myfilename);
        qr.bindValue("currentsize",currentsize);
        qr.bindValue("totalsize",totalsize);
        qr.bindValue("mime",mymime);
        if(priority < 0)qr.bindValue("tstatus",0);
        else qr.bindValue("tstatus",-100);
        qr.bindValue("categoryid",catId);
        qr.bindValue("priority",2);
        qr.bindValue("note",gui->textEdit->document()->toPlainText());

    }
    else
    {
        qr.prepare("INSERT INTO tasks(url,filename,datecreate,tstatus,categoryid,priority,note) VALUES(:url,:filename,:datecreate,:tstatus,:categoryid,:priority,:note);");
        qr.bindValue("url", gui->urlBox->currentText());
        qr.bindValue("filename",flname);
        qr.bindValue("datecreate",dtime.toString("yyyy-MM-ddThh:mm:ss"));
        if(priority < 0)qr.bindValue("tstatus",0);
        else qr.bindValue("tstatus",-100);
        qr.bindValue("categoryid",catId);
        qr.bindValue("priority",2);
        qr.bindValue("note",gui->textEdit->document()->toPlainText());
    }
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

void AddTaskDialog::acceptQAction(QAbstractButton *btn)
{
    QPointer<EMessageBox> dlg = qobject_cast<EMessageBox*>(sender());
    if(!dlg)return;

    switch(dlg->myTypes())
    {
    case EMessageBox::AT_REDOWNLOAD:
    {
        if(dlg->buttonRole(btn) == EMessageBox::RejectRole) {close();return;}
        else
        {
            QSqlQuery qr(mydb);
            qr.prepare("DELETE FROM tasks WHERE url=:url;");
            qr.bindValue("url",gui->urlBox->currentText());

            if(!qr.exec())
            {
                //тут запись в журнал ошибок
                qDebug()<<"void AddTaskDialog::acceptQAction(1): Error: "<<qr.lastError().text();
                close();
                return;
            }
        }
        addTask();
        return;
    }
    default: return;
    }
}

void AddTaskDialog::setAdditionalInfo(const QString &flnm, qint64 cursz, qint64 totalsz, const QString &mime)
{
    myfilename = flnm;
    currentsize = cursz;
    totalsize = totalsz;
    mymime = mime;
    additional_flag = true;
    getCategory(myfilename);
}

void AddTaskDialog::getCategory(const QString &file)
{
    QString fl;
    if(additional_flag && gui->urlBox->currentText() == defUrl)fl = myfilename;
    else fl = file;
    QUrl nurl = QUrl::fromEncoded(fl.toAscii());
    QFileInfo flinfo(nurl.toString(QUrl::RemoveFragment | QUrl::RemoveQuery));

    QSqlQuery qr(mydb), qr1(mydb);
    qr.prepare("SELECT id,extlist FROM categories WHERE extlist LIKE :ext1 OR extlist LIKE :ext2 OR extlist LIKE :ext3 OR extlist LIKE :ext4");
    qr1.prepare("SELECT id,extlist FROM categories WHERE extlist LIKE '%\%U%' ESCAPE '\\' ");
    QString ext_ = flinfo.suffix().toLower();
    qr.bindValue("ext1", QString("% %1 %").arg(ext_));
    qr.bindValue("ext2", QString("%1 %").arg(flinfo.suffix()));
    qr.bindValue("ext3", QString("% %1").arg(flinfo.suffix()));
    qr.bindValue("ext4", QString("%1").arg(flinfo.suffix()));
    if(!qr.exec())
    {
        //тут запись в журнал ошибок
        qDebug()<<"void AddTaskDialog::getCategory(1): Error: "<<qr.lastError().text();
        close();
        return;
    }
    if(!qr1.exec())
    {
        //тут запись в журнал ошибок
        qDebug()<<"void AddTaskDialog::getCategory(2): Error: "<<qr1.lastError().text();
        close();
        return;
    }

    int catId = 6;
    int chars_max_cnt = 0;
    while(qr1.next())
    {
        QStringList extlst = qr1.value(1).toString().split(" "); //получаем список расширений и хостов
        for(int i = 0; extlst.indexOf(QRegExp("%U[:./\\w+]+"),i) >= 0;)
        {
            int index = extlst.indexOf(QRegExp("%U[:./\\w+]+"),i);
            int chars_cnt = extlst.value(index).size();
            if(chars_cnt <= chars_max_cnt)continue;
            QString curhost = extlst.value(index).right(chars_cnt-2);
            if(fl.indexOf(curhost,0,Qt::CaseInsensitive) >= 0)
            {
                chars_max_cnt = chars_cnt;
                catId = qr1.value(0).toInt();
            }
            i = index + 1;
        }
    }
    if(catId == 6 && qr.next())
        catId = qr.value(0).toInt();

    gui->categoryBox->setCurrentIndex(gui->categoryBox->findData(catId));
}
