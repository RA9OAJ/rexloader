#define P_VERSION "0.1a.2"
#define P_BUILD_DATE "2011-05-26"

#include "httploader.h"

HttpLoader::HttpLoader(QObject *parent) : QObject(parent)
{
    task_list = new QHash<int, Task*>;
    sections = new QHash<HttpSection*, int>;
    squeue = new QList<int>;
    t_mutex = new QMutex();
    q_mutex = new QMutex();
    del_queue = new QList<HttpSection*>;
    maxTaskNum = 0;
    shedule_flag = false;
    fullsize_res = false; //резервировать место под файл целиком
    attempt_interval = 3000;

    translator = new QTranslator(this);
    QString slocale = ":/lang/";
    slocale += QLocale::system().name();
    if(!translator->load(slocale))translator->deleteLater();
    else QApplication::installTranslator(translator);
}

HttpLoader::~HttpLoader()
{
    deleteTask(0);
    task_list->clear();
    sections->clear();
    squeue->clear();
    while(!del_queue->isEmpty())scanDelQueue();
    delete(task_list);
    delete(sections);
    delete(squeue);
    delete(t_mutex);
    delete(q_mutex);
    delete(del_queue);
}

QStringList HttpLoader::protocols() const
{
    return QStringList() << "http" << "https";
}

QStringList HttpLoader::pluginInfo() const
{
    QStringList pinfo;
    pinfo << QString("Authors: ") + tr("Sarvaritdinov Ravil (mailto:ra9oaj@mail.ru)");
    pinfo << QString("Place: ") + tr("Russia, Barabinsk, 2011");
    pinfo << QString("Build date: ") + QString(P_BUILD_DATE);
    pinfo << QString("Version: ") + QString(P_VERSION);
    pinfo << QString("Copyright: ") + tr("© 2011 Sarvaritdinov Ravil");
    pinfo << QString("Lic: ") + tr("GNU GPL 2");
    pinfo << QString("Description: ") + tr("Plugin for downloading files on HTTP.");
    return pinfo;
}

int HttpLoader::addTask(const QUrl &_url)
{
    if(_url.isEmpty() || !_url.isValid())return 0; //если URL не валиден или пуст
    t_mutex->lock();
    Task *tsk = 0;
    tsk = new Task();
    if(!tsk){t_mutex->unlock();return 0;}
    tsk->url = _url;
    tsk->_fullsize_res = fullsize_res;
    tsk->_maxSections = this->maxSections;
    int task_num = 0;
    if(!task_list->key(0))
        task_num = task_list->size()+1;
    else task_num = task_list->key(0);
    task_list->insert(task_num, tsk);
    t_mutex->unlock();
    return task_num;
}

void HttpLoader::deleteTask(int id_task)
{
    t_mutex->lock();
    if(!id_task) //в случае остановки и удаления всех заданий
    {
        QList<HttpSection*> _sections = sections->keys();
        for(int i=0; i<_sections.size(); i++)
                _sections.value(i)->stopDownloading();

        /*for(int i=0; i<_sections.size(); i++)
            _sections.value(i)->wait();*/

        shedule_flag = false;
        t_mutex->unlock();
        return;
    }
    if(!task_list->contains(id_task)){t_mutex->unlock(); return;}
    if(task_list->value(id_task) == 0){t_mutex->unlock(); return;}
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
    t_mutex->unlock();

    mathSpeed(); //пересчитываем скорость на каждую секцию
}

void HttpLoader::setDownSpeed(long long _spd)
{
    speed = _spd;
    mathSpeed();
}

void HttpLoader::mathSpeed()
{
    t_mutex->lock(); //блочим изменения в заданиях
    int as_cnt = sections->size(); //счетчик активных секций

    if(!as_cnt){t_mutex->unlock(); return;} //если нет активных секций, то и пересчитывать нечего, выходим из метода

    qint64 sect_spd = speed/(qint64)as_cnt;
    //HttpSection *sect;
    QList<HttpSection*> lst = sections->keys();
    for(int i = 0; i<lst.size(); i++)
        lst.value(i)->setDownSpeed(sect_spd);

    t_mutex->unlock();
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
    return task_list->value(id_task)->accept_ranges;
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
    qint64 cur_size = tsk->totalLoad();
    int elapsed_ = tsk->watcher.elapsed();
    if(!elapsed_ ||elapsed_ < 50) return tsk->cur_speed;
    qint64 spd = (cur_size - tsk->last_size) / (qint64)elapsed_ * 1000;
    tsk->watcher.start();
    tsk->last_size = cur_size;
    if(spd < 0 )return 0;
    tsk->cur_speed = spd;
    return spd;
}

long long int HttpLoader::totalDownSpeed() const
{
    qint64 total_spd = 0;

    QList<int> lst = task_list->keys();
    for(int i = 0; i<lst.size(); ++i)
    {
        if(!task_list->value(lst.value(i)))continue;
        total_spd += downSpeed(lst.value(i));
    }

    if(total_spd < 0)return 0;
    return total_spd;
}

void HttpLoader::startDownload(int id_task)
{
    if(!task_list->contains(id_task) || taskStatus(id_task) != LInterface::ON_PAUSE)return;
    if(!task_list->value(id_task))return;

    Task *tsk = task_list->value(id_task);

    HttpSection *sect = new HttpSection();
    sect->setUrlToDownload(tsk->url.toString());
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
    connect(sect,SIGNAL(downloadingCompleted()),this,SLOT(sectionCompleted())/*, Qt::QueuedConnection*/);
    connect(sect,SIGNAL(acceptRanges()),this,SLOT(acceptRang())/*, Qt::QueuedConnection*/);

    int sect_id = 0;
    if(tsk->map[0] || tsk->map[2] || tsk->map[4] || tsk->map[6] || tsk->map[8] || tsk->map[10]) //если уже распределены границы, то загружаем данные о первой недокачанной секции
    {
        for(int i = 1; i < 7; i++)
        {
            qint64 end_s = tsk->map[2*i] != 0 ? tsk->map[2*i]:tsk->map[12];
            if(end_s > tsk->map[2*i-2] + tsk->map[2*i-1])
            {
                sect->setSection(tsk->map[2*i-2], tsk->map[2*i]-1);
                if(tsk->map[2*i-1])sect->setOffset(tsk->map[2*i-1]);
                sect_id = i;
                break;
            }
        }
    }
    if(!sect_id)sect_id = 1;

    t_mutex->lock();
    tsk->sections_cnt += 1;
    tsk->sections.insert(sect_id,sect);
    sections->insert(sect, id_task);
    t_mutex->unlock();
    mathSpeed();
    if(!shedule_flag) //если шедулер не работает
    {
        shedule_flag = true; //разрешаем ему работу
        sheduler(); //запускаем шедулер
    }
    tsk->watcher.start();
    tsk->status = LInterface::SEND_QUERY;
    sect->startDownloading(); //запускаем секцию
}

void HttpLoader::stopDownload(int id_task)
{
    if(!task_list->contains(id_task) || taskStatus(id_task) == LInterface::ON_PAUSE)return;
    if(!task_list->value(id_task))return;

    Task *tsk = task_list->value(id_task);
    HttpSection *sect;
    t_mutex->lock();
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
    t_mutex->unlock();
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
    t_mutex->lock();
    Task* tsk = getTaskSender(sender());
    if(!tsk){t_mutex->unlock();return;}
    if(!tsk->size){tsk->size = _sz; tsk->map[12]=_sz;t_mutex->unlock();return;}
    if(tsk->size != _sz)
    {
        tsk->status = LInterface::ERROR_TASK;
        tsk->error_number = LInterface::FILE_SIZE_ERROR;
        stopDownload(task_list->key(tsk));
    }
    t_mutex->unlock();
}

void HttpLoader::redirectToUrl(const QString &_url)
{
    HttpSection *sect = 0;
    QObject *_sender = sender();
    if(!_sender)return;
    sect = qobject_cast<HttpSection*>(_sender);
    if(!sect || !sections->contains(sect)) return;

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

    if(!tsk) // если задание-владелец секции не найден, то секция тупо удаляется
    {
        //if(!sect->isFinished())sect->stopDownloading();
        //while(!sect->wait(1000));
        sections->remove(sect);
        addDeleteQueue(sect);
        //sect->quit();
        //sect->deleteLater();
        sect = 0;
        mathSpeed();
        return;
    }
    int id_task = task_list->key(tsk);

    qint64 _total = (sect->finishByte() == 0 && sect->startByte() == 0) ? tsk->totalLoad() : (sect->finishByte() != 0 ? sect->finishByte()-sect->startByte()+1:sect->totalFileSize()-sect->startByte());

    if(sect->totalLoadOnSection() == _total && _total > 0) //если закачка прошла успешно
    {

        t_mutex->lock();
        //if(!sect->isFinished())sect->stopDownloading();
        //while(!sect->isFinished());
        tsk->sections.remove(tsk->sections.key(sect));
        sections->remove(sect);
        addDeleteQueue(sect);
        //sect->quit();
        //sect->deleteLater();
        sect = 0;
        tsk->sections_cnt -= 1;
        if(tsk->status == LInterface::SEND_QUERY)tsk->status = LInterface::ON_LOAD;

        if(tsk->totalLoad() == tsk->size)
        {
            QFile tmpfl(tsk->filepath);
            tmpfl.resize(tsk->size);
            tsk->status = LInterface::FINISHED;
            t_mutex->unlock();
            mathSpeed();
            return;
        }
        t_mutex->unlock();
        mathSpeed();
    }
    else if(sect->totalLoadOnSection() < _total || !_total) //если скачано меньше, чем нужно или сервер не передал общего размера файла
        {
            t_mutex->lock();
            //if(!sect->isFinished())sect->stopDownloading();
            //while(!sect->isFinished());
            tsk->sections.remove(tsk->sections.key(sect));
            sections->remove(sect);
            //sect->deleteLater();
            addDeleteQueue(sect);
            //sect->quit();
            sect = 0;
            tsk->sections_cnt -= 1;
        }

    t_mutex->unlock();
    mathSpeed();

    if(tsk->status != LInterface::STOPPING) tsk->sections_cnt == 0 ? addSection(id_task) : QTimer::singleShot(1000,this,SLOT(addSection()));

}

void HttpLoader::syncFileMap(Task* _task)
{
    if(!_task->accept_ranges)return; //если докачка не поддерживается, то выходим
    QFileInfo flinfo(_task->filepath);
    if(!QFile::exists(_task->filepath) || !flinfo.isFile()) return;

    FILE *fl = fopen(_task->filepath.toAscii().data(), "rb+");
    if(!fl)
    {
        _task->status = LInterface::ERROR_TASK;
        _task->error_number = LInterface::FILE_WRITE_ERROR;
        int id_task = task_list->key(_task);
        stopDownload(id_task);
        return;
    }

    //bool _erflag = false;
    qint64 spos = 0;
    if(!_task->_fullsize_res || !_task->size)
    {
        /*QList<int> _keys = _task->sections.keys();
        int maxid = 0;
        for(int i = 0; i<_keys.size(); ++i)
            maxid = qMax(maxid, _keys.value(i));
        spos = _task->map[2*maxid-2]+_task->map[2*maxid-1];*/
        for(int i=11; i>=1; --i)
            if(_task->map[i])
            {
                spos = _task->map[i-1]+_task->map[i];
                break;
            }
    }
    else spos = _task->size;

    fseek(fl, spos, SEEK_SET);
    QByteArray outbuf("\r\nRExLoader 0.1a.1\r\n");
    fwrite(outbuf.data(), 1, outbuf.size(), fl);
    int _lenght = _task->url.toString().toAscii().size();
    fwrite(&_lenght, 1, sizeof(int),fl); //длина строки с URL
    fwrite(_task->url.toString().toAscii().data(), 1, _task->url.toString().toAscii().size(), fl); //строка с URL
    _lenght = _task->referer.toAscii().size();
    fwrite(&_lenght, 1, sizeof(int), fl); //длина строки реферера, если нет, то 0
    if(_lenght) fwrite(_task->referer.toAscii().data(), 1, _task->referer.toAscii().size(), fl);
    _lenght = _task->MIME.toAscii().size();
    fwrite(&_lenght, 1, sizeof(int), fl); //длина строки с MIME-типом файла
    if(_lenght) fwrite(_task->MIME.toAscii().data(), 1, _task->MIME.toAscii().size(), fl);
    qint64 total_ = _task->size;
    fwrite(&total_, 1, sizeof(qint64), fl);
    for(int i = 0; i < 13; i++)
        fwrite(&_task->map[i], 1, sizeof(qint64), fl);
    QString lastmodif = _task->last_modif.toString("yyyy-MM-ddTHH:mm:ss");
    _lenght = lastmodif.size();
    fwrite(&_lenght, 1, sizeof(int), fl); //длина строки с датой
    if(_lenght) fwrite(lastmodif.toAscii().data(), 1, _lenght, fl);
    fwrite(&spos, sizeof(qint64), 1,fl); //метка начала блока метаданных

    fclose(fl);
}

void HttpLoader::addSection()
{
    q_mutex->lock();
    if(squeue->isEmpty()){q_mutex->unlock(); return;}
    int id_task = squeue->takeFirst();
    if(!task_list->contains(id_task)){q_mutex->unlock(); return;}
    Task* tsk = task_list->value(id_task);
    if(!tsk){q_mutex->unlock(); return;}
    q_mutex->unlock();

    addSection(id_task);
}

void HttpLoader::addSection(int id_task)
{
    t_mutex->lock();
    if(!task_list->contains(id_task)){t_mutex->unlock(); return;}
    Task* _task = task_list->value(id_task);
    if(!_task){t_mutex->unlock(); return;}

    if(_task->sections_cnt >= _task->_maxSections) //проверяем на лимит секций
    {
        t_mutex->unlock();
        _task->status = LInterface::ON_LOAD;
        return;
    }

    int cur_sect_id = 0;
    for(int i = 1; i < 7; i++) //находим недокачанную и не запущенную секцию
    {
        qint64 cur_sect = _task->map[2*i-2] + _task->map[2*i-1];
        qint64 end_sect = _task->map[2*i] != 0 ? _task->map[2*i]:_task->map[12];
        if((cur_sect < end_sect || !end_sect) && !_task->sections.contains(i)){cur_sect_id = i;break;}
    }

    if(!cur_sect_id){_task->status = LInterface::ON_LOAD;t_mutex->unlock(); return;} // если все секции задействованы, то выходим

    HttpSection *sect = new HttpSection();
    sect->setUrlToDownload(_task->url.toString());
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

    sect->setSection(_task->map[2*cur_sect_id-2], (_task->map[2*cur_sect_id] == 0 ? 0 : _task->map[2*cur_sect_id]-1)); //устанавливаем границы секции

    if(_task->map[2*cur_sect_id-1])sect->setOffset(_task->map[2*cur_sect_id-1]);

    _task->sections.insert(cur_sect_id, sect);
    ++_task->sections_cnt;
    sections->insert(sect,id_task);

    t_mutex->unlock();
    if(!shedule_flag) //если шедулер не работает
    {
        shedule_flag = true; //разрешаем ему работу
        sheduler(); //запускаем шедулер
    }
    mathSpeed();
    sect->startDownloading(); //запускаем секцию

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
        tsk->status = LInterface::ERROR_TASK; //в случае критичных ошибок
        stopDownload(id_task);
        break;
    case HttpSection::SERV_CONNECT_ERROR:
    case QAbstractSocket::ConnectionRefusedError:
    case QAbstractSocket::SocketTimeoutError:
    case QAbstractSocket::ProxyConnectionRefusedError:
    case QAbstractSocket::ProxyConnectionClosedError:
    case QAbstractSocket::ProxyConnectionTimeoutError:
    case 403:
    case 503:
        t_mutex->lock();
        if(tsk->sections_cnt < 2) //если этот сигнал получен от единственной секции
        {
            tsk->status = LInterface::ERROR_TASK; //в случае критичных ошибок
            tsk->error_number = er;
            t_mutex->unlock();
            stopDownload(id_task);
            mathSpeed();
            break;
        }

        ++tsk->errors_cnt;
        if(tsk->errors_cnt == maxErrors)
        {
            tsk->error_number = LInterface::ERROR_TASK;
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
        }
        t_mutex->unlock();
        mathSpeed();
        break;

    case 416:
        if(!tsk->size)
        {
            t_mutex->lock();
            QFile tmpfl(tsk->filepath);
            tmpfl.resize(sect->totalLoadOnSection());
            //if(sect->isRunning())sect->wait(5000);
            tsk->sections.remove(tsk->sections.key(sect));
            sections->remove(sect);
            addDeleteQueue(sect);
            --tsk->sections_cnt;
            sect = 0;
            tsk->status = LInterface::FINISHED;
            t_mutex->unlock();
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
    q_mutex->lock();
    squeue->append(id_task);
    q_mutex->unlock();

    tsk->status = LInterface::SEND_QUERY;
    QTimer::singleShot(1000,this,SLOT(addSection()));
}

void HttpLoader::setAttemptInterval(const int sec)
{
    if(sec < 1) return;
    attempt_interval = sec;
}

int HttpLoader::loadTaskFile(const QString &_path)
{
    QFileInfo flinfo(_path);
    if(!QFile::exists(_path) || !flinfo.isFile())return 0;

    FILE *fl = fopen(_path.toAscii().data(), "rb");
    if(!fl)return 0;

    qint64 spos = 0;

    if(fseek(fl, flinfo.size()-8, SEEK_SET) != 0)return 0;
    fread(&spos, sizeof(qint64), 1, fl);
    if(fseek(fl, spos, SEEK_SET) != 0)return 0;

    QString header;
    QByteArray buffer;
    buffer.resize(1024);
    fgets(buffer.data(), 1024, fl);
    header.append(buffer);
    if(header != "\r\n")return 0;
    header.clear();
    fgets(buffer.data(), 1024, fl);
    header.append(buffer);
    //if(header != "RExLoader\r\n")return 0;
    if(header.indexOf("RExLoader")!= 0)return 0;
    QString fversion = header.split(" ").value(1);
    if(fversion != "0.1a\r\n")return 0;

    int length = 0;
    fread(&length, sizeof(int), 1, fl);
    buffer.resize(length);
    fread(buffer.data(),length, 1, fl); //считываем URL


    Task *tsk = 0;
    tsk = new Task();
    if(!tsk){t_mutex->unlock();return 0;}
    tsk->url = QUrl(QString(buffer));
    tsk->_fullsize_res = fullsize_res;
    tsk->_maxSections = this->maxSections;
    tsk->filepath = _path;

    length = 0;
    fread(&length, sizeof(int), 1, fl);
    buffer.resize(length);
    fread(buffer.data(),length, 1, fl); //считываем реферера
    tsk->referer = buffer;

    length = 0;
    fread(&length, sizeof(int), 1, fl);
    buffer.resize(length);
    fread(buffer.data(),length, 1, fl); //считываем MIME
    tsk->MIME = buffer;

    fread(&tsk->size, sizeof(qint64), 1, fl); //считываем общий размер задания

    for(int i=0; i<14; ++i)
    {
        fread(&tsk->map[i], sizeof(qint64), 1, fl); //считывание карты секций
    }

    length = 0;
    fread(&length, sizeof(int), 1, fl);
    buffer.resize(length);
    fread(buffer.data(),length, 1, fl); //считываем дату модификации
    tsk->last_modif = QDateTime::fromString(QString(buffer),"yyyy-MM-ddTHH:mm:ss");

    int task_num = 0;
    t_mutex->lock();
    if(!task_list->key(0))
        task_num = task_list->size()+1;
    else task_num = task_list->key(0);
    task_list->insert(task_num, tsk);
    t_mutex->unlock();

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
    Task* tsk = getTaskSender(sender());
    t_mutex->lock();
    if(!tsk){t_mutex->unlock(); return;}
    tsk->accept_ranges = true;
    if(tsk->_maxSections == 1){t_mutex->unlock(); return;}
    HttpSection* sect = qobject_cast<HttpSection*>(sender());
    if(!sect){t_mutex->unlock(); return;}
    int sect_id = tsk->sections.key(sect);
    if(!sect_id){t_mutex->unlock(); return;}
    tsk->status = LInterface::ON_LOAD;
    tsk->filepath = sect->fileName(); //задаем имя локального файла, пределенное при запросе к серверу
    int id_task = task_list->key(tsk);

    //---Тут анализ и разбивка общего объема на секции---// + Должно быть дополнительное условие на проверку не распределения задания на секции
    if(!sect->totalFileSize() || tsk->map[2] != 0){tsk->status=LInterface::ON_LOAD;t_mutex->unlock(); addSection(id_task); return;}
    //if(!sect->totalFileSize()){t_mutex->unlock(); return;}
    qint64 cur_spd = qMin(sect->realSpeed(), sect->downSpeed());
    cur_spd = cur_spd != 0 ? cur_spd : 100*1024*1024;

   // disconnect(sect,SIGNAL(downloadingCompleted()),this,SLOT(sectionCompleted()));
    disconnect(sect,SIGNAL(acceptRanges()),this,SLOT(acceptRang()));

    if(tsk->size / cur_spd <= 10){tsk->_maxSections = 1; t_mutex->unlock(); return;}
    sect->stopDownloading();

    //while(!sect->wait(1000)){qDebug()<<"HttpLoader While 4"; /*sect->terminate();*/}
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

    t_mutex->unlock();

    sect->setSection(tsk->map[0], tsk->map[2]-1);
    sect->setOffset(tsk->map[1]);
    //connect(sect,SIGNAL(downloadingCompleted()),this,SLOT(sectionCompleted())/*, Qt::QueuedConnection*/);
    connect(sect, SIGNAL(acceptQuery()),this,SLOT(acceptQuery()));
    mathSpeed();

    /*sections->remove(sect);
    tsk->sections.remove(sect_id);
    --tsk->sections_cnt;
    addDeleteQueue(sect);
    sect = 0;
    int id_task = task_list->key(tsk);
    addSection(id_task);*/
    //connect(sect,SIGNAL(finished()), sect,SLOT(startDownloading()));
    sect->startDownloading();

}

void HttpLoader::addDeleteQueue(HttpSection *sect_)
{
    del_queue->append(sect_);
    //sect_->stopDownloading();
    //sect_->quit();
}

void HttpLoader::scanDelQueue()
{
    for(int i = 0; i<del_queue->size(); ++i)
    {
        if(!del_queue->value(i)->freedMemory())continue;
        disconnect(this,SIGNAL(sheduleImpulse()),del_queue->value(i),SLOT(transferActSlot()));
        //disconnect(del_queue->value(i),SIGNAL(finished()),del_queue->value(i),SLOT(startDownloading()));
        //connect(del_queue->value(i),SIGNAL(finished()),del_queue->value(i),SLOT(deleteLater()));
        //del_queue->value(i)->quit();
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

    default: errstr = tr("Unknown Error. Error code = ") + QString::number(_err); break;
    }

    return errstr;
}

QString HttpLoader::statusString(int _stat) const
{
    _stat = 0;
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

Q_EXPORT_PLUGIN2(HttpLoader, HttpLoader)
