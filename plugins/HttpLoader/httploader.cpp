/*
Project: REXLoader (Downloader, plugin: HttpLoader), Source file: httploader.cpp
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

#define P_VERSION "0.1a.3"
#define P_BUILD_DATE "2012-05-24"

#include "httploader.h"

HttpLoader::HttpLoader(QObject *parent)
{
    Q_UNUSED(parent)
    task_list = new QHash<int, Task*>;
    sections = new QHash<HttpSection*, int>;
    squeue = new QList<int>;
    dqueue = new QList<int>;
    del_queue = new QList<HttpSection*>;
    aqueue = new QList<QObject*>;
    maxTaskNum = 0;
    shedule_flag = false;
    fullsize_res = false; //резервировать место под файл целиком
    ignore_critical = 0;
    maxErrors = 5;
    attempt_interval = 3000;
}

HttpLoader::~HttpLoader()
{
    deleteTask(0);
    task_list->clear();
    sections->clear();
    squeue->clear();
    aqueue->clear();
    while(!del_queue->isEmpty())scanDelQueue();
    delete task_list;
    delete sections;
    delete squeue;
    delete dqueue;
    delete del_queue;
    delete aqueue;
}

QStringList HttpLoader::protocols() const
{
    return QStringList() << "http" << "https";
}

QStringList HttpLoader::pluginInfo() const
{
    QStringList pinfo;
    pinfo << QString("Plugin: ") + tr("HttpLoader");
    pinfo << QString("Authors: ") + tr("Sarvaritdino R.");
    pinfo << QString("Place: Russia, Barabinsk, 2011-2012");
    pinfo << QString("Build date: ") + QString(P_BUILD_DATE);
    pinfo << QString("Version: ") + QString(P_VERSION);
    pinfo << QString("Contacts: mailto:ra9oaj@mail.ru");
    pinfo << QString("Lic: GNU/LGPL v2.1");
    pinfo << QString("Description: ") + tr("Плагин для скачивания файлов по протоколу HTTP/S.");
    return pinfo;
}

int HttpLoader::addTask(const QUrl &_url)
{
    if(_url.isEmpty() || !_url.isValid())return 0; //если URL не валиден или пуст
    Task *tsk = 0;
    tsk = new Task();
    if(!tsk)return 0;
    tsk->url = _url;
    tsk->_fullsize_res = fullsize_res;
    tsk->_maxSections = this->maxSections;
    int task_num = 0;
    if(!task_list->key(0))
        task_num = task_list->size()+1;
    else task_num = task_list->key(0);
    task_list->insert(task_num, tsk);
    return task_num;
}

int HttpLoader::countTask() const
{
    return task_list->size();
}

void HttpLoader::deleteTask(int id_task)
{
    if(!id_task) //в случае остановки и удаления всех заданий
    {
        QList<HttpSection*> _sections = sections->keys();
        for(int i=0; i<_sections.size(); i++)
                _sections.value(i)->stopDownloading();

        /*for(int i=0; i<_sections.size(); i++)
            _sections.value(i)->wait();*/

        shedule_flag = false;
        return;
    }
    if(!task_list->contains(id_task))return;
    if(task_list->value(id_task) == 0)return;
    QList<HttpSection*> _sections = sections->keys(id_task);
    if(!_sections.isEmpty()) //если у задания есть активные секции
    {
        for(int i = 0; i<_sections.size(); i++)
        {

            _sections.value(i)->stopDownloading();
            int sect_id = task_list->value(id_task)->sections.key(_sections.value(i));
            task_list->value(id_task)->sections.remove(sect_id);
            --task_list->value(id_task)->sections_cnt;
            addDeleteQueue(_sections.value(i));
            //_sections.value(i)->wait(1000);
            sections->remove(_sections.value(i));
        }
        //while(task_list->value(id_task)->sections_cnt != 0)/*qDebug()<<"HttpLoader While 1"*/; //пока все секции не будут сохранены крутимся тут
        delete task_list->value(id_task); //удаляем задание из памяти
    }

    if(id_task == task_list->size()) //если id_task последний добавленный элемент хэша
    task_list->remove(id_task); //просто удаляем его
    else task_list->insert(id_task, 0); //иначе обнуляем указатель на объект задания

    mathSpeed(); //пересчитываем скорость на каждую секцию
}

void HttpLoader::setDownSpeed(long long _spd)
{
    speed = _spd;
    mathSpeed();
}

void HttpLoader::mathSpeed()
{
    int as_cnt = sections->size(); //счетчик активных секций

    if(!as_cnt) return; //если нет активных секций, то и пересчитывать нечего, выходим из метода

    qint64 sect_spd = speed/(qint64)as_cnt;
    //HttpSection *sect;
    QList<HttpSection*> lst = sections->keys();
    for(int i = 0; i<lst.size(); i++)
        lst.value(i)->setDownSpeed(sect_spd);
}

void HttpLoader::setMaxSectionsOnTask(int _max)
{
    maxSections = _max;
}

void HttpLoader::setUserAgent(const QString &_uagent)
{
    uAgent = _uagent;
}

void HttpLoader::setAuthorizationData(int id_task, const QString &data_base64)
{
    if(!task_list->contains(id_task))return;
    task_list->value(id_task)->authData = data_base64;
}

void HttpLoader::setReferer(int id_task, const QString &uref)
{
    if(!task_list->contains(id_task))return;
    task_list->value(id_task)->referer = uref;
}

long long int HttpLoader::totalSize(int id_task) const
{
    if(!task_list->contains(id_task))return LInterface::NO_TASK;
    if(!task_list->value(id_task))return LInterface::NO_TASK;
    return task_list->value(id_task)->size;
}

long long int HttpLoader::sizeOnSection(int id_task, int _sect_num) const
{
    if(!task_list->contains(id_task))return LInterface::NO_TASK;
    if(!task_list->value(id_task))return LInterface::NO_TASK;
    Task *tsk = task_list->value(id_task);
    if(!tsk->sections.contains(_sect_num))return LInterface::NO_SECTION;
    qint64 _start = 0;
    qint64 _finish = 0;
    _start = tsk->sections.value(_sect_num)->startByte();
    _finish = tsk->sections.value(_sect_num)->finishByte();
    return _finish - _start;
}

long long int HttpLoader::totalLoadedOnTask(int id_task) const
{
    if(!task_list->contains(id_task))return LInterface::NO_TASK;
    if(!task_list->value(id_task))return LInterface::NO_TASK;
    return task_list->value(id_task)->totalLoad();
}

long long int HttpLoader::totalLoadedOnSection(int id_task, int _sect_num) const
{
    if(!task_list->contains(id_task))return LInterface::NO_TASK;
    if(!task_list->value(id_task))return LInterface::NO_TASK;
    Task *tsk = task_list->value(id_task);
    if(!tsk->sections.contains(_sect_num))return LInterface::NO_SECTION;
    return tsk->sections.value(_sect_num)->totalLoadOnSection();
}

int HttpLoader::taskStatus(int id_task) const
{
    if(!task_list->contains(id_task))return LInterface::NO_TASK;
    if(!task_list->value(id_task))return LInterface::NO_TASK;
    return task_list->value(id_task)->status;
}

bool HttpLoader::acceptRanges(int id_task) const
{
    if(!task_list->contains(id_task))return false;
    if(!task_list->value(id_task))return false;
    //Доработка ISS#15
    return (task_list->value(id_task)->accept_ranges);
}

QString HttpLoader::mimeType(int id_task) const
{
    if(!task_list->contains(id_task))return false;
    if(!task_list->value(id_task))return false;
    return (task_list->value(id_task)->MIME);
}

int HttpLoader::errorNo(int id_task) const
{
    if(!task_list->contains(id_task))return LInterface::NO_TASK;
    if(!task_list->value(id_task))return LInterface::NO_TASK;
    return task_list->value(id_task)->error_number;
}

long long int HttpLoader::downSpeed(int id_task) const
{
    if(!task_list->contains(id_task))return LInterface::NO_TASK;
    if(!task_list->value(id_task))return LInterface::NO_TASK;
    Task* tsk = task_list->value(id_task);
    qint64 spd = 0;
    QList<HttpSection*> sect_lst = tsk->sections.values();
    for(int i = 0; i < sect_lst.size(); ++i)
    {
        if(!sect_lst.value(i))continue;
        spd += sect_lst.value(i)->realSpeed();
    }
    return spd;
}

long long int HttpLoader::totalDownSpeed() const
{
    qint64 total_spd = 0;

    QList<HttpSection*> sectlist = sections->keys();
    for(int i = 0; i < sectlist.size(); ++i)
    {
        if(!sectlist.value(i))continue;
        total_spd += sectlist.value(i)->realSpeed();
    }
    return total_spd;
}

void HttpLoader::startDownload(int id_task)
{
    if(!task_list->contains(id_task) || taskStatus(id_task) != LInterface::ON_PAUSE)return;
    if(!task_list->value(id_task))return;

    Task *tsk = task_list->value(id_task);

    HttpSection *sect = new HttpSection();
    sect->setUrlToDownload(QString(tsk->url.toEncoded()));
    sect->setFileName(tsk->filepath);
    if(!tsk->authData.isEmpty())sect->setAuthorizationData(tsk->authData);
    sect->setUserAgent(uAgent);
    if(!tsk->referer.isEmpty())sect->setReferer(tsk->referer);
    sect->setLastModified(tsk->last_modif);

    if(tsk->proxy_type != LInterface::PROXY_NOPROXY)
    {
        QNetworkProxy::ProxyType proxytype;
        switch(tsk->proxy_type)
        {
        case LInterface::PROXY_HTTP: proxytype = QNetworkProxy::HttpProxy; break;
        case LInterface::PROXY_SOCKS5: proxytype = QNetworkProxy::Socks5Proxy; break;
        case LInterface::PROXY_DEFAULT: proxytype = QNetworkProxy::DefaultProxy; break;
        default: proxytype = QNetworkProxy::NoProxy; break;
        }

        sect->setProxy(tsk->proxy, proxytype, tsk->proxy_auth);
    }

    connect(this,SIGNAL(sheduleImpulse()),sect,SLOT(transferActSlot())/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(errorSignal(int)),this,SLOT(sectError(int))/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(transferCompleted(qint64)),this,SLOT(acceptSectionData())/*, Qt::QueuedConnection*/);
    //connect(sect,SIGNAL(acceptQuery()),this,SLOT(acceptQuery()));
    connect(sect,SIGNAL(totalSize(qint64)),this,SLOT(setTotalSize(qint64))/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(redirectToUrl(QString)),this,SLOT(redirectToUrl(QString))/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(fileType(QString)),this,SLOT(setMIME(QString))/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(mismatchOfDates(QDateTime,QDateTime)),this,SLOT(mismatchOfDates(QDateTime,QDateTime))/*, Qt::QueuedConnection*/);
    //connect(sect,SIGNAL(downloadingCompleted()),this,SLOT(sectionCompleted())/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(acceptRanges()),this,SLOT(addInAQueue())/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(sectionMessage(int,QString,QString)),this,SLOT(addMessage(int,QString,QString)));
    connect(sect,SIGNAL(rangeNotAccepted()),this,SLOT(makeSingleSection()));

    int sect_id = 0;
    if(tsk->map[0] || tsk->map[1] || tsk->map[2] || tsk->map[4] || tsk->map[6] || tsk->map[8] || tsk->map[10]) //если уже распределены границы, то загружаем данные о первой недокачанной секции
    {
        for(int i = 1; i < 7; i++)
        {
            qint64 end_s = tsk->map[2*i] != 0 ? tsk->map[2*i]:tsk->map[12];
            if(end_s > tsk->map[2*i-2] + tsk->map[2*i-1])
            {
                sect->setSection(tsk->map[2*i-2], tsk->map[2*i-2]+tsk->map[2*i-1]+1/*(tsk->map[2*i] == 0 ? 0 : tsk->map[2*i]-1)*/);

                if(tsk->map[2*i-1])sect->setOffset(tsk->map[2*i-1]);
                sect_id = i;
                break;
            }
        }
    }
    else sect->setSection(0,1);
    if(!sect_id)sect_id = 1;

    tsk->sections_cnt += 1;
    tsk->sections.insert(sect_id,sect);
    sections->insert(sect, id_task);
    mathSpeed();
    int task_id = sections->value(sect,0);
    emit messageAvailable(task_id,sect_id,LInterface::MT_INFO,tr("Установлены границы секции с %1 байта по %2 байт").arg(QString::number(sect->startByte()),QString::number(sect->finishByte())),QString());
    emit messageAvailable(task_id,sect_id,LInterface::MT_INFO,tr("Установлено смещение в секции на %1").arg(QString::number(sect->totalLoadOnSection())),QString());

    if(!shedule_flag) //если шедулер не работает
    {
        shedule_flag = true; //разрешаем ему работу
        sheduler(); //запускаем шедулер
    }
    tsk->watcher.start();
    tsk->status = LInterface::SEND_QUERY;
    sect->setCookie(tsk->cookie);
    sect->startDownloading(); //запускаем секцию
}

void HttpLoader::stopDownload(int id_task)
{
    if(!task_list->contains(id_task) || taskStatus(id_task) == LInterface::ON_PAUSE)return;
    if(!task_list->value(id_task))return;

    Task *tsk = task_list->value(id_task);
    HttpSection *sect;
    int last_status = tsk->status;
    tsk->status = LInterface::STOPPING;
    QList<int> keys = tsk->sections.keys();
    for(int i = 0; i<keys.count(); i++)
    {
        sect = 0;
        sect = tsk->sections.value(keys[i]);
        sect->stopDownloading();
        addDeleteQueue(sect);
        //while(!sect->wait(1000));
        sections->remove(sect);
        tsk->sections.remove(keys[i]);
        tsk->sections_cnt -= 1;
    }

    tsk->status = last_status;
    if(tsk->status != LInterface::ERROR_TASK)tsk->status = LInterface::ON_PAUSE;
    mathSpeed();
    if(!sections->size())shedule_flag = false; //останавливаем шедулер, если нет активных секций
}

void HttpLoader::sheduler()
{
    scanDelQueue();
    if(!shedule_flag)
    {
        if(del_queue->size() > 0)QTimer::singleShot(50,this,SLOT(sheduler()));
        return;
    }
    emit sheduleImpulse();
    QTimer::singleShot(50,this,SLOT(sheduler()));
}

Task* HttpLoader::getTaskSender(QObject* _sender) const
{
    HttpSection* sect = qobject_cast<HttpSection*>(_sender);
    if(!sect)return 0;
    if(!sect || !sections->contains(sect)) return 0;
    int id_task = sections->value(sect);
    return task_list->value(id_task);
}

void HttpLoader::setTotalSize(qint64 _sz)
{
    Task* tsk = getTaskSender(sender());
    if(!tsk) return;
    if(!tsk->size){tsk->size = _sz; tsk->map[12]=_sz;return;}
    if(tsk->size != _sz)
    {
        tsk->status = LInterface::ERROR_TASK;
        tsk->error_number = LInterface::FILE_SIZE_ERROR;
        stopDownload(task_list->key(tsk));
    }
}

void HttpLoader::redirectToUrl(const QString &_url)
{
    HttpSection *sect = 0;
    QObject *_sender = sender();
    if(!_sender)return;
    sect = qobject_cast<HttpSection*>(_sender);
    if(!sect || !sections->contains(sect)) return;

    Task *tsk = getTaskSender(sender());
    tsk->mirrors.insert(-1, QUrl::fromEncoded(_url.toAscii()));
    // Исправление ошибки с именем файла при редиректе
    QFileInfo flinfo(tsk->filepath);
    if(!flinfo.exists() && !flinfo.isDir() && flinfo.absoluteDir().exists())
        sect->setFileName(flinfo.absoluteDir().absolutePath());
    //------------------------------

    sect->setUrlToDownload(_url);
    sect->startDownloading();
}

void HttpLoader::setMIME(const QString &_mime)
{
    Task *tsk = getTaskSender(sender());
    if(!tsk || !tsk->MIME.isEmpty())return;
    tsk->MIME = _mime;
}

void HttpLoader::mismatchOfDates(const QDateTime &_last, const QDateTime &_cur)
{
    HttpSection *sect = qobject_cast<HttpSection*>(sender());
    if(!sect)return;
    Task* tsk = getTaskSender(sender());
    if(!tsk)return;
    int id_task = task_list->key(tsk);

    tsk->err_modif = _cur;
    if(tsk->last_modif.isNull())tsk->last_modif = _last;
    tsk->error_number = LInterface::FILE_DATETIME_ERROR;
    stopDownload(id_task);

    return;
}

void HttpLoader::sectionCompleted()
{
    HttpSection *sect = 0;
    QObject *_sender = sender();
    if(!_sender)return;
    sect = qobject_cast<HttpSection*>(_sender);
    if(!sect || !sections->contains(sect)) return;
    Task* tsk = getTaskSender(sender());
    int task_id = sections->value(sect,0);

    if(!tsk) // если задание-владелец секции не найден, то секция тупо удаляется
    {
        sections->remove(sect);
        addDeleteQueue(sect);
        sect = 0;
        mathSpeed();
        return;
    }
    int id_task = task_list->key(tsk);
    qint64 _total = (sect->finishByte() == 0 && sect->startByte() == 0) ? tsk->size : (sect->finishByte() != 0 ? sect->finishByte()-sect->startByte()+1:sect->totalFileSize()-sect->startByte());
    if(tsk->filepath != sect->fileName())tsk->filepath = sect->fileName();
    if(sect->totalLoadOnSection() == _total && _total > 0) //если закачка прошла успешно
    {
        tsk->sections.remove(tsk->sections.key(sect));
        sections->remove(sect);
        addDeleteQueue(sect);
        sect = 0;
        tsk->sections_cnt -= 1;
        if(tsk->status == LInterface::SEND_QUERY)tsk->status = LInterface::ON_LOAD;
        if(task_id) emit messageAvailable(task_id, tsk->sections.key(sect),LInterface::MT_INFO,tr("Секция завершена"),QString());

        if(tsk->totalLoad() == tsk->size || (!tsk->size && tsk->MIME.split("/").value(0).toLower() == "text"))
        {
            QFile tmpfl(tsk->filepath);
            if(!tsk->size)tsk->size = _total;
            tmpfl.resize(tsk->size);
            tsk->status = LInterface::FINISHED;
            mathSpeed();

            return;
        }
        mathSpeed();
    }
    else if(sect->totalLoadOnSection() < _total || !_total) //если скачано меньше, чем нужно или сервер не передал общего размера файла
    {
        if(!tsk->accept_ranges)
        {
            tsk->status = LInterface::FINISHED;
            tsk->size = tsk->totalLoad();
            mathSpeed();
            return;
        }
        tsk->sections.remove(tsk->sections.key(sect));
        sections->remove(sect);
        addDeleteQueue(sect);
        sect = 0;
        tsk->sections_cnt -= 1;
    }
    mathSpeed();

    if(task_id) emit messageAvailable(task_id, tsk->sections.key(sect),LInterface::MT_INFO,tr("Проверка размера задания"),QString());
    if(tsk->status != LInterface::STOPPING) tsk->sections_cnt == 0 ? addSection(id_task) : QTimer::singleShot(1000,this,SLOT(addSection()));

}

void HttpLoader::syncFileMap(Task* _task)
{
    if(!_task->accept_ranges)return; //если докачка не поддерживается, то выходим
    QFileInfo flinfo(_task->filepath);
    if(!QFile::exists(_task->filepath) || !flinfo.isFile()) return;

    QFile fl(_task->filepath);
    if(!fl.open(QIODevice::ReadWrite))
    {
        _task->status = LInterface::ERROR_TASK;
        _task->error_number = LInterface::FILE_WRITE_ERROR;
        int id_task = task_list->key(_task);
        stopDownload(id_task);
        return;
    }

    qint64 spos = 0;
    if(!_task->_fullsize_res || !_task->size)
    {
        for(int i=11; i>=1; --i)
            if(_task->map[i])
            {
                spos = _task->map[i-1]+_task->map[i];
                break;
            }
    }
    else spos = _task->size;

    fl.seek(spos);
    QDataStream file(&fl);
    QByteArray outbuf("\r\nRExLoader 0.1a.1\r\n");
    file.writeRawData(outbuf.data(),outbuf.size());
    int _lenght = _task->url.toEncoded().size();
    file << (qint32)_lenght; //длина строки с URL
    file.writeRawData(_task->url.toEncoded().data(), _task->url.toEncoded().size()); //строка с URL
    _lenght = _task->referer.toAscii().size();
    file << (qint32)_lenght; //длина строки реферера, если нет, то 0
    if(_lenght) file.writeRawData(_task->referer.toAscii().data(),_task->referer.toAscii().size());
    _lenght = (qint32)_task->MIME.toAscii().size();
    file << (qint32)_lenght; //длина строки с MIME-типом файла
    if(_lenght) file.writeRawData(_task->MIME.toAscii().data(), _task->MIME.toAscii().size());
    qint64 total_ = _task->size;
    file << total_;
    for(int i = 0; i < 13; i++)
        file << _task->map[i];
    QString lastmodif = _task->last_modif.toString("yyyy-MM-ddTHH:mm:ss");
    _lenght = lastmodif.size();
    file << (qint32)_lenght; //длина строки с датой
    if(_lenght) file.writeRawData(lastmodif.toAscii().data(), _lenght);
    file << spos; //метка начала блока метаданных

    fl.close();
}

void HttpLoader::addSection()
{
    if(squeue->isEmpty()) return;
    int id_task = squeue->takeFirst();
    if(!task_list->contains(id_task)) return;
    Task* tsk = task_list->value(id_task);
    if(!tsk) return;

    if(!tsk->accept_ranges)
        tsk->accept_ranges = true;
    addSection(id_task);
}

void HttpLoader::addSection(int id_task)
{
    if(!task_list->contains(id_task)) return;
    Task* _task = task_list->value(id_task);
    if(!_task) return;
    if(_task->status == LInterface::FINISHED) return;

    if(_task->sections_cnt >= _task->_maxSections) //проверяем на лимит секций
    {
        _task->status = LInterface::ON_LOAD;
        return;
    }

    int cur_sect_id = 0;

    for(int i = 1; i < 7; i++) //находим недокачанную и не запущенную секцию
    {
        if(i != 1 && _task->map[2*i-2] == 0)continue; //25.07.2011
        qint64 cur_sect = _task->map[2*i-2] + _task->map[2*i-1];
        qint64 end_sect = _task->map[2*i] != 0 ? _task->map[2*i]:_task->map[12];
        if((cur_sect < end_sect || !end_sect) && !_task->sections.contains(i)){cur_sect_id = i;break;}
    }

    if(!cur_sect_id){_task->status = LInterface::ON_LOAD; return;} // если все секции задействованы, то выходим

    QUrl _url = _task->mirrors.contains(-1) ? _task->mirrors.value(-1) : _task->url;
    HttpSection *sect = new HttpSection();
    sect->setUrlToDownload(_url.toString());
    sect->setFileName(_task->filepath);
    if(!_task->authData.isEmpty())sect->setAuthorizationData(_task->authData);
    sect->setUserAgent(uAgent);
    if(!_task->referer.isEmpty())sect->setReferer(_task->referer);
    sect->setLastModified(_task->last_modif);

    if(_task->proxy_type != LInterface::PROXY_NOPROXY)
    {
        QNetworkProxy::ProxyType proxytype;
        switch(_task->proxy_type)
        {
        case LInterface::PROXY_HTTP: proxytype = QNetworkProxy::HttpProxy; break;
        case LInterface::PROXY_SOCKS5: proxytype = QNetworkProxy::Socks5Proxy; break;
        case LInterface::PROXY_DEFAULT: proxytype = QNetworkProxy::DefaultProxy; break;
        default: proxytype = QNetworkProxy::NoProxy; break;
        }

        sect->setProxy(_task->proxy, proxytype, _task->proxy_auth);
    }

    connect(this,SIGNAL(sheduleImpulse()),sect,SLOT(transferActSlot())/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(errorSignal(int)),this,SLOT(sectError(int))/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(transferCompleted(qint64)),this,SLOT(acceptSectionData())/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(redirectToUrl(QString)),this,SLOT(redirectToUrl(QString))/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(acceptQuery()),this,SLOT(acceptQuery())/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(mismatchOfDates(QDateTime,QDateTime)),this,SLOT(mismatchOfDates(QDateTime,QDateTime))/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(downloadingCompleted()),this,SLOT(sectionCompleted())/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(sectionMessage(int,QString,QString)),this,SLOT(addMessage(int,QString,QString)));

    _task->sections.insert(cur_sect_id, sect);
    ++_task->sections_cnt;
    sections->insert(sect,id_task);

    sect->setSection(_task->map[2*cur_sect_id-2], (_task->map[2*cur_sect_id] == 0 ? 0 : _task->map[2*cur_sect_id]-1)); //устанавливаем границы секции

    if(_task->map[2*cur_sect_id-1])sect->setOffset(_task->map[2*cur_sect_id-1]);

    if(!shedule_flag) //если шедулер не работает
    {
        shedule_flag = true; //разрешаем ему работу
        sheduler(); //запускаем шедулер
    }
    mathSpeed();
    sect->setCookie(_task->cookie);
    sect->startDownloading(); //запускаем секцию

}

void HttpLoader::setMaxErrorsOnTask(const int max)
{
    if(max < 0)
        maxErrors = 0;
    else
        maxErrors = max;
}

void HttpLoader::setRetryCriticalError(const bool flag)
{
    ignore_critical = flag;
}

void HttpLoader::addRetSection()
{
    if(dqueue->isEmpty()) return;
    int id_task = dqueue->takeFirst();
    if(!task_list->contains(id_task)) return;
    Task* tsk = task_list->value(id_task);
    if(!tsk) return;

    addSection(id_task);
}

void HttpLoader::makeSingleSection()
{
    HttpSection *sect = qobject_cast<HttpSection*>(sender());
    if(!sect) return;
    Task *tsk = task_list->value(sections->value(sect));

    sect->stopDownloading();
    tsk->accept_ranges = false;
    tsk->clearMap();

    sect->setSection(0,0);
    sect->setOffset(0);
    connect(sect,SIGNAL(downloadingCompleted()),this,SLOT(sectionCompleted()));
    disconnect(sect,SIGNAL(acceptRanges()),this,SLOT(addInAQueue()));
    sect->startDownloading();
}

void HttpLoader::addMessage(int ms_type, const QString &message, const QString &more)
{
    HttpSection *sect = qobject_cast<HttpSection*>(sender());
    if(!sect) return;
    Task *tsk = task_list->value(sections->value(sect));
    int task_id = sections->value(sect,0);
    if(task_id) emit messageAvailable(task_id, tsk->sections.key(sect),ms_type,message,more);
}

void HttpLoader::sectError(int _errno)
{
    Task* tsk = getTaskSender(sender());
    HttpSection* sect = qobject_cast<HttpSection*>(sender());
    if(!sect || !tsk)return;
    int id_task = task_list->key(tsk);
    int er = 0;

    switch(_errno)
    {
    case HttpSection::SIZE_ERROR: tsk->error_number = LInterface::FILE_SIZE_ERROR; break;
    case HttpSection::DATE_ERROR: tsk->error_number = LInterface::FILE_DATETIME_ERROR; break;
    case HttpSection::WRITE_ERROR: tsk->error_number = LInterface::FILE_WRITE_ERROR; break;
    case HttpSection::FILE_NOT_AVAILABLE: tsk->error_number = HttpSection::FILE_NOT_AVAILABLE; break;
    case QAbstractSocket::HostNotFoundError: tsk->error_number = LInterface::HOST_NOT_FOUND; break;
    case QAbstractSocket::NetworkError: tsk->error_number = LInterface::CONNECT_LOST; break;
    case QAbstractSocket::ProxyNotFoundError: tsk->error_number = LInterface::PROXY_NOT_FOUND; break;
    case QAbstractSocket::ProxyAuthenticationRequiredError: tsk->error_number = LInterface::PROXY_AUTH_ERROR; break;
    case QAbstractSocket::ProxyProtocolError: tsk->error_number = LInterface::PROXY_ERROR; break;
    case 404: tsk->error_number = LInterface::FILE_NOT_FOUND; break;
    case HttpSection::SERV_CONNECT_ERROR: er = LInterface::CONNECT_ERROR; break;
    case QAbstractSocket::ConnectionRefusedError: er = LInterface::CONNECT_ERROR; break;
    case QAbstractSocket::SocketTimeoutError: er = LInterface::CONNECT_ERROR; break;
    case QAbstractSocket::ProxyConnectionRefusedError: er = LInterface::PROXY_CONNECT_REFUSED; break;
    case QAbstractSocket::ProxyConnectionClosedError: er = LInterface::PROXY_CONNECT_CLOSE; break;
    case QAbstractSocket::ProxyConnectionTimeoutError: er = LInterface::PROXY_TIMEOUT; break;
    case 403:
    case 503:
    default: er = _errno; break;
    }

    //!!!!--------------------------------

    switch(_errno)
    {
    case HttpSection::SIZE_ERROR:
    case HttpSection::DATE_ERROR:
    case HttpSection::WRITE_ERROR:
    case QAbstractSocket::HostNotFoundError:
    case QAbstractSocket::NetworkError:
    case QAbstractSocket::ProxyNotFoundError:
    case QAbstractSocket::ProxyAuthenticationRequiredError:
    case QAbstractSocket::ProxyProtocolError:
    case 404:
        if(tsk->sections_cnt < 2)
        {
            tsk->status = LInterface::ERROR_TASK; //в случае критичных ошибок
            stopDownload(id_task);
            break;
        }
    case HttpSection::SERV_CONNECT_ERROR:
    case HttpSection::FILE_NOT_AVAILABLE:
    case QAbstractSocket::RemoteHostClosedError:
    case QAbstractSocket::ConnectionRefusedError:
    case QAbstractSocket::SocketTimeoutError:
    case QAbstractSocket::ProxyConnectionRefusedError:
    case QAbstractSocket::ProxyConnectionClosedError:
    case QAbstractSocket::ProxyConnectionTimeoutError:
    case 400:
    case 403:
    case 409:
    case 503:

        if(tsk->sections_cnt < 2)++tsk->errors_cnt;
        if(tsk->errors_cnt >= maxErrors)
        {
            tsk->status = LInterface::ERROR_TASK;
            tsk->error_number = LInterface::ERRORS_MAX_COUNT;
            stopDownload(id_task);
        }
        else
        {
            tsk->sections.remove(tsk->sections.key(sect));
            sections->remove(sect);
            addDeleteQueue(sect);
            --tsk->sections_cnt;
            if(tsk->status == LInterface::SEND_QUERY)tsk->status = LInterface::ON_LOAD;
            sect = 0;

            dqueue->append(id_task);
            QTimer::singleShot(attempt_interval,this,SLOT(addRetSection()));
        }
        mathSpeed();
        break;

    case 416:
        if(!tsk->size)
        {
            QFile tmpfl(tsk->filepath);
            tmpfl.resize(sect->totalLoadOnSection());
            tsk->sections.remove(tsk->sections.key(sect));
            sections->remove(sect);
            addDeleteQueue(sect);
            --tsk->sections_cnt;
            sect = 0;
            tsk->status = LInterface::FINISHED;
            break;
        }

    default:
        tsk->status = LInterface::ERROR_TASK;
        tsk->error_number = _errno;
        break;
    }

}

void HttpLoader::acceptSectionData()
{
    Task* tsk = getTaskSender(sender());
    if(!tsk)return;
    HttpSection* sect = qobject_cast<HttpSection*>(sender());
    if(!sect)return;

    int sect_id = tsk->sections.key(sect);
    tsk->map[2*sect_id-1] = sect->totalLoadOnSection();
    if(!sect->lastModified().isNull() && sect->lastModified().isValid() && tsk->last_modif.isNull())
        tsk->last_modif = sect->lastModified();
    tsk->map[13] = tsk->totalLoad();

    bool ismax = false;
    QList<int> _keys = tsk->sections.keys();
    int maxid = 0;
    for(int i = 0; i<_keys.size(); ++i)
        maxid = qMax(maxid, _keys.value(i));
    ismax = maxid == sect_id ? true : false;

    if(tsk->_fullsize_res)
    {
        if(ismax)
        {
            sect->pauseDownloading(true);
            syncFileMap(tsk);
            sect->pauseDownloading(false);
        }
    }
    else syncFileMap(tsk);


    switch(tsk->status)
    {
    case LInterface::ACCEPT_QUERY:
    case LInterface::REDIRECT:
        tsk->status = LInterface::ON_LOAD;
        return;
        break;
    case LInterface::ERROR_TASK:
    default:
        return;
        break;
    }

}

void HttpLoader::acceptQuery()
{

    mathSpeed();
    Task* tsk = getTaskSender(sender());
    if(!tsk)return;
    if(tsk->status == LInterface::SEND_QUERY)tsk->status = LInterface::ACCEPT_QUERY;
    int id_task = task_list->key(tsk);

    if(tsk->sections_cnt == tsk->_maxSections){tsk->status = LInterface::ON_LOAD;return;}
    squeue->append(id_task);

    tsk->status = LInterface::SEND_QUERY;
    QTimer::singleShot(attempt_interval,this,SLOT(addSection()));
}

void HttpLoader::setAttemptInterval(const int sec)
{
    if(sec < 1) return;
    attempt_interval = sec*1000;
}

int HttpLoader::loadTaskFile(const QString &_path)
{
    QFileInfo flinfo(_path);
    if(!QFile::exists(_path) || !flinfo.isFile())return 0;

    QFile fl(_path);
    if(!fl.open(QIODevice::ReadOnly))return 0;
    QDataStream file(&fl);

    qint64 spos = 0;
    if(!fl.seek(flinfo.size() - 8)){fl.close(); return 0;}
    file >> spos;
    if(!fl.seek(spos)){fl.close(); return 0;}

    QString header;
    header = fl.readLine(3);
    if(header != "\r\n"){fl.close(); return 0;}
    header.clear();
    header = fl.readLine(254);
    if(header.indexOf("RExLoader")!= 0){fl.close(); return 0;}
    QString fversion = header.split(" ").value(1);
    if(fversion != "0.1a.1\r\n"){fl.close(); return 0;}

    int length = 0;
    file >> length;
    QByteArray buffer;
    buffer.resize(length);
    file.readRawData(buffer.data(),length); //считываем URL

    Task *tsk = 0;
    tsk = new Task();
    if(!tsk) {fl.close(); return 0;}
    tsk->url = QUrl::fromEncoded(buffer);
    tsk->_fullsize_res = fullsize_res;
    tsk->_maxSections = this->maxSections;
    tsk->filepath = _path;

    length = 0;
    file >> length;
    buffer.resize(length);
    file.readRawData(buffer.data(),length); //считываем реферера
    tsk->referer = buffer;

    length = 0;
    file >> length;
    buffer.resize(length);
    file.readRawData(buffer.data(),length); //считываем MIME
    tsk->MIME = buffer;

    file >> tsk->size; //считываем общий размер задания

    for(int i=0; i<13; ++i)
        {
            file >> tsk->map[i]; //считывание карты секций
        }

    length = 0;
    file >> length;
    buffer.resize(length);
    file.readRawData(buffer.data(),length); //считываем дату модификации
    tsk->last_modif = QDateTime::fromString(QString(buffer),"yyyy-MM-ddTHH:mm:ss");
    fl.close();

    int task_num = 0;
    if(!task_list->key(0))
        task_num = task_list->size()+1;
    else task_num = task_list->key(0);
    task_list->insert(task_num, tsk);

    return task_num;
}

int HttpLoader::countSectionTask(int id_task) const
{
    if(!task_list->contains(id_task))return 0;
    if(!task_list->value(id_task))return 0;
    return task_list->value(id_task)->sections_cnt;
}

void HttpLoader::setTaskFilePath(int id_task, const QString &_path)
{
    if(!task_list->contains(id_task))return;
    task_list->value(id_task)->filepath = _path;
}

void HttpLoader::acceptRang()
{
    if(aqueue->isEmpty())return;
    QObject* _sender = aqueue->takeFirst();
    Task* tsk = getTaskSender(_sender);/*getTaskSender(sender());*/
    if(!tsk)return;
    HttpSection* sect = qobject_cast<HttpSection*>(_sender);
    if(!sect)return;
    int sect_id = tsk->sections.key(sect);
    if(!sect_id)return;
    int id_task = task_list->key(tsk);
    if(!id_task)return;

    //---Тут анализ и разбивка общего объема на секции---
    if(!sect->totalFileSize() || tsk->map[2] != 0)
    {
        tsk->status=LInterface::ON_LOAD;
        addSection(id_task);
        return;
    }

    if(100*tsk->totalLoad()/tsk->size >= 50 && (tsk->map[2]+tsk->map[4]+tsk->map[6]+tsk->map[8]+tsk->map[10]==0))
    {
        tsk->_maxSections = 1;
        return;
    }
    sect->stopDownloading();

    qint64 sect_size = tsk->size / tsk->_maxSections;
    qint64 load_section = tsk->map[2*sect_id-1];

    for(int i = 1; i < tsk->_maxSections; ++i)
    {
        tsk->map[2*i]=sect_size*i;
        if(load_section > sect_size*i)
        {
            tsk->map[2*i-1] = sect_size;
            if(load_section - sect_size*i < sect_size)
                tsk->map[2*i+1] = load_section - sect_size;
        }
    }

    QUrl _url = tsk->mirrors.contains(-1) ? tsk->mirrors.value(-1) : tsk->url;
    sect->setUrlToDownload(QString(_url.toEncoded()));
    sect->setSection(tsk->map[0], tsk->map[2]-1);
    sect->setOffset(tsk->map[1]);
    mathSpeed();
    sect->startDownloading();
}

void HttpLoader::addDeleteQueue(HttpSection *sect_)
{
    del_queue->append(sect_);
}

void HttpLoader::scanDelQueue()
{
    for(int i = 0; i<del_queue->size(); ++i)
    {
        if(!del_queue->value(i)->freedMemory())continue;
        disconnect(this,SIGNAL(sheduleImpulse()),del_queue->value(i),SLOT(transferActSlot()));
        int index = aqueue->lastIndexOf(del_queue->value(i));
        if(index != -1)aqueue->replace(index,0);
        del_queue->value(i)->deleteLater();
        del_queue->removeOne(del_queue->value(i));
    }
}

QString HttpLoader::taskFilePath(int id_task) const
{
    if(!task_list->contains(id_task))return QString();
    return task_list->value(id_task)->filepath;
}

QString HttpLoader::errorString(int _err) const
{
    QString errstr;

    switch(_err)
    {
    case 400: errstr = tr("400 Bad Request"); break;
    case 401: errstr = tr("401 Unauthorized"); break;
    case 403: errstr = tr("403 Forbidden"); break;
    case 407: errstr = tr("407 Proxy Authentication Required"); break;
    case 409: errstr = tr("409 Conflict"); break;
    case 410: errstr = tr("410 Gone"); break;
    case 411: errstr = tr("414 Request-URL Too Long"); break;
    case 500: errstr = tr("500 Internal Server Error"); break;
    case 501: errstr = tr("501 Not Implemented"); break;
    case 502: errstr = tr("502 Bad Gateway"); break;
    case 503: errstr = tr("503 Service Unavailable"); break;
    case 504: errstr = tr("504 Gateway Timeout"); break;
    case 505: errstr = tr("505 HTTP Version Not Supported"); break;
    case HttpSection::FILE_NOT_AVAILABLE: errstr = tr("Файл больше недоступен по данному URL"); break;

    default: errstr = tr("Неизвестная ошибка. Код ошибки = ") + QString::number(_err); break;
    }

    return errstr;
}

QString HttpLoader::statusString(int _stat) const
{
    //_stat = 0;
    Q_UNUSED(_stat)
    return "";
}

void HttpLoader::setProxy(int id_task, const QUrl &_proxy, LInterface::ProxyType _ptype, const QString &data_base64)
{
    if(!task_list->contains(id_task))return;
    Task *tsk = task_list->value(id_task);
    tsk->proxy = _proxy;
    tsk->proxy_type = _ptype;
    tsk->proxy_auth = data_base64;
}

void HttpLoader::setAdvancedOptions(int id_task, const QString &options)
{
    if(!task_list->contains(id_task))return;
    QStringList opts = options.split("\n\n");
    QString par;
    foreach(par,opts)
    {
        QStringList curopt = par.split("cookie:");
        if(curopt.size() > 1)
        {
            task_list->value(id_task)->cookie = curopt.value(1);
            continue;
        }

        curopt.clear();
        curopt = par.split("referer:");
        if(curopt.size() > 1)
        {
            task_list->value(id_task)->referer = curopt.value(1);
            continue;
        }
    }
}

void HttpLoader::addInAQueue()
{
    aqueue->append(sender());

    Task* tsk = getTaskSender(sender());
    if(!tsk)return;
    tsk->accept_ranges = true;
    HttpSection* sect = qobject_cast<HttpSection*>(sender());
    if(!sect)return;
    int sect_id = tsk->sections.key(sect);
    if(!sect_id)return;
    tsk->status = LInterface::ON_LOAD;
    tsk->filepath = sect->fileName();

    disconnect(sect,SIGNAL(acceptRanges()),this,SLOT(addInAQueue()));
    connect(sect, SIGNAL(acceptQuery()),this,SLOT(acceptQuery()));
    connect(sect,SIGNAL(downloadingCompleted()),this,SLOT(sectionCompleted()));
    sect->setSection(tsk->map[2*sect_id-2], (tsk->map[2*sect_id] == 0 ? 0 : tsk->map[2*sect_id]-1));
    if(tsk->map[2*sect_id-1]) sect->setOffset(tsk->map[2*sect_id-1]);
    sect->startDownloading();
    if(tsk->_maxSections == 1)return;

    QTimer::singleShot(5000,this,SLOT(acceptRang()));
}

QTranslator* HttpLoader::getTranslator(const QLocale &locale)
{
    translator = new QTranslator();
    QString slocale = ":/lang/";
    slocale += locale.name();
    if(!translator->load(slocale))
    {
        translator->deleteLater();
        translator = 0;
    }

    return translator;
}

Q_EXPORT_PLUGIN2(HttpLoader, HttpLoader)
