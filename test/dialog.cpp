#include "dialog.h"
#include "ui_dialog.h"

#define TEST 0

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    QString plug = QApplication::applicationDirPath()+"/libHttpLoader.so";
    qDebug()<<QApplication::applicationDirPath();
    QFile ft(plug);
    if(ft.exists())qDebug()<<"YES";
    QPluginLoader ploader(plug);
    ploader.load();
    qDebug()<<ploader.errorString();
    if(!ploader.isLoaded())qDebug()<<"NotLoad!!!";
    loader = qobject_cast<LoaderInterface*>(ploader.instance());
    if(!loader){QTimer::singleShot(100, this,SLOT(close())); return;}
    qDebug()<<loader->protocols();
    qDebug()<<loader->pluginInfo();
    task_id = 0;
    ui->filePath->setText(QDir::homePath());
    dir_ = QDir::homePath();
    ui->progressBar->setValue(0);
    total = loaded = 0;
    ui->Stop->setEnabled(false);
    last_spd = 0;

    connect(ui->browse, SIGNAL(released()),this,SLOT(showDirBrowser()));
    connect(ui->start,SIGNAL(released()),this,SLOT(startDownload()));
    connect(ui->Stop,SIGNAL(released()),this,SLOT(stopDownload()));
    connect(ui->spinBox,SIGNAL(valueChanged(int)),this,SLOT(setSpeed(int)));
    loader->setDownSpeed(ui->spinBox->value()*1024/8);
    ui->url->setStyleSheet("border-style: solid; border-width: 1px; border-color: red;");
    scanTaskStatus();
}

void Dialog::scanTaskStatus()
{
    QString s1;
    QFile fl(ui->filePath->text());
    if(task_id)
    {
        if(loader->taskFilePath(task_id) != ui->filePath->text())
        {
            ui->filePath->setText(loader->taskFilePath(task_id));
            fl.setFileName(ui->filePath->text());
        }

        int stat = loader->taskStatus(task_id);
        //qDebug()<<stat;
        switch(stat)
        {
        case LInterface::ON_PAUSE:
            ui->status->setText("Остановлено");
            break;
        case LInterface::ON_LOAD:
            ui->status->setText("Закачивается");
            break;
        case LInterface::SEND_QUERY:
            s1 = QString("Посылка запроса [%1]").arg(loader->countSectionTask(task_id)+1);
            ui->status->setText(s1);
            break;
        case LInterface::ACCEPT_QUERY:
            ui->status->setText("Запрос принят...");
            break;
        case LInterface::ERROR_TASK:
            ui->status->setText("Ошибка");
            QMessageBox::critical(this,"Ошибка при скачивании",QString("Ошибка: %1").arg(QString::number(loader->errorNo(task_id))));
            stopDownload();
            break;
        case LInterface::REDIRECT:
            ui->status->setText("Секция перенаправлена");
            break;
        case LInterface::FINISHED:
            ui->status->setText("Задание завершено");
            ui->filePath->setText(loader->taskFilePath(task_id));
            //fl.resize(total);
            stopDownload();
            break;
        default: break;
        }

        if(loader->taskStatus(task_id) != LInterface::SEND_QUERY)ui->section_cnt->setText(QString::number(loader->countSectionTask(task_id)));
        else ui->section_cnt->setText(QString::number(loader->countSectionTask(task_id)));
        qint64 cur = loader->downSpeed(task_id);


        cur = loader->downSpeed(task_id);
       //if(cur < 0)return;
        if(total && cur > 0)
        {
            qint64 sec = (total-loader->totalLoadedOnTask(task_id))/cur;
            int days = sec/3600/24;
            sec -= (qint64)days*3600*24;
            int hours = sec/3600;
            sec -= (qint64)hours*3600;
            int minuts = sec/60;
            sec -= (qint64)minuts*60;
            QString time_="";
            if(days)time_ = time_ + QString("%1д. ").arg(QString::number(days));
            if(days || hours)time_ = time_ + QString("%1час. ").arg(QString::number(hours));
            if(hours || minuts)time_ = time_ + QString("%1мин. ").arg(QString::number(minuts));
            time_ += QString("%1сек.").arg(QString::number(sec));
            ui->time->setText(time_);
        }
        else {
            ui->time->setText("Неизвестно");
        }

        cur = cur/1024;
        if(cur - last_spd > 20) {last_spd = cur;}
        else
        {
            QString s = "";
            if(cur >= 1024) s = QString("%1 MB/s").arg(QString::number((double)cur/1024.0,'f',1));
            else s = QString("%1 KB/s").arg(QString::number(cur));
            ui->speed->setText(s);
            last_spd = cur;
        }

        if(!total)
        {
            total = loader->totalSize(task_id);
            if(total > 0)ui->progressBar->setMaximum((int)(total/1024/1024));
            else total = 0;
        }

        loaded = loader->totalLoadedOnTask(task_id);
        if(loaded)ui->progressBar->setValue((int)(loaded/1024/1024));


    }
    QTimer::singleShot(500,this,SLOT(scanTaskStatus()));
}

void Dialog::startDownload()
{
    if(task_id != 0)loader->deleteTask(task_id);
    QUrl myurl(ui->url->text());
    if(!myurl.isValid())
    {
        ui->url->setStyleSheet("border-color: red;");
        return;
    }
    loader->setMaxSectionsOnTask(1);
    task_id = /*loader->loadTaskFile("/home/rav/1.mp4");*/loader->addTask(myurl);
    if(!task_id)return;
    QFileInfo info(ui->url->text());
    QString path = ui->filePath->text();
    /*if(path[path.size()-1] != '/')path += "/";
    if(!myurl.hasQuery())path += info.fileName();*/
    ui->filePath->setText(path);

    for(int i = 0; i<TEST; i++)
    {
        pool[i] = loader->addTask(myurl);
        if(!pool[i])continue;
        loader->setTaskFilePath(pool[i],path+QString::number(i));
        loader->startDownload(pool[i]);
    }

    //loader->setDownSpeed(15*1024*1024);
    loader->setTaskFilePath(task_id, path);
    //loader->setProxy(task_id, QUrl("proxy://109.230.216.23:1080/"),LInterface::PROXY_SOCKS5,"");
    loader->startDownload(task_id);

    ui->Stop->setEnabled(true);
    ui->start->setEnabled(false);
    ui->url->setReadOnly(true);
    ui->browse->setEnabled(false);
}

void Dialog::stopDownload()
{
    if(loader->taskStatus(task_id) != LInterface::FINISHED)loader->stopDownload(task_id);
    for(int i=0; i<TEST; ++i)
    {
        loader->stopDownload(pool[i]);
        loader->deleteTask(pool[i]);
        pool[i]=0;
    }
    //while(loader->countSectionTask(task_id) > 0);/*{loader->stopDownload(task_id);qDebug()<<"Blya!!"<<loader->countSectionTask(task_id);}*/
    loader->deleteTask(task_id);
    task_id = 0;

    ui->Stop->setEnabled(false);
    ui->start->setEnabled(true);
    ui->url->setReadOnly(false);
    ui->filePath->setText(dir_);
    ui->browse->setEnabled(true);
}

void Dialog::_test()
{
   // qDebug()<<"test: "<<sender()->objectName();
}

void Dialog::showDirBrowser()
{
    if(task_id) loader->deleteTask(task_id);
    QString dir = QFileDialog::getExistingDirectory(this,"Директория для сохранения","~/");
    if(dir.isEmpty())return;
    ui->filePath->setText(dir);
    dir_ = dir;
}

void Dialog::setSpeed(int _spd)
{
    qint64 spd = 1024*_spd/8;
    //qDebug()<<spd;
    loader->setDownSpeed(spd);
}

Dialog::~Dialog()
{
    delete ui;
}
