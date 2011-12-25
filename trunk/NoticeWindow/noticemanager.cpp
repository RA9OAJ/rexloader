/*
<one line to give the program's name and a brief idea of what it does.>
Copyright (C) <year>  <name of author>

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

#include "noticemanager.h"

NoticeManager::NoticeManager(QObject *parent) :
    QObject(parent)
{
    list.clear();
    queue.clear();
    notices.clear();
    maxnotices = 5;
    all_desktop = true;
    notice_size = QSize(250,120);
    dif = QSize(3,3);
    effect = NoticeWindow::SE_PopUp;
}

void NoticeManager::showNotice(const QString &title, const QString &message, NoticeWindow::WindowType type)
{
    NoticeWindow *wnd = new NoticeWindow();
    wnd->setAttribute(Qt::WA_DeleteOnClose);
    wnd->setMaximumSize(notice_size);
    wnd->setDisplayTime(5);
    connect(wnd,SIGNAL(rejected()),this,SLOT(closeOneWindow()));
    if(list.size() < maxnotices)
    {
        list.append(wnd);

        if(!(list.size()-1)) wnd->setOffsetPos(dif.width(),dif.height());
        else
        {
            wnd->setDOffsetPos(0,(list.size()-1)*notice_size.height()+dif.height()+(list.size()-1)-1);
            wnd->setOffsetPos(dif.width(),1);
        }
        wnd->showNotice(title,message,type);
    }
    else
    {
        QStringList str;
        str << title << message;
        queue.append(QPair<QStringList,NoticeWindow::WindowType>(str,type));
    }
}

void NoticeManager::closeOneWindow()
{
    NoticeWindow *wnd = qobject_cast<NoticeWindow*>(sender());
    if(!wnd)return;
    int id = list.indexOf(wnd);

    if(id < 0)return;
    list.removeOne(wnd);

    if(list.size() > 0)
    {
        for(int i = id; i < list.size(); i++)
        {
            if(list.value(i)->size().height() < notice_size.height())
            {
                list.value(i)->setDOffsetPos(0,(i != 0) ? (i*notice_size.height()+dif.height()+i-1) : 0);
                if(!i)list.value(i)->setOffsetPos(dif.width(),dif.height());
            }
            else list.value(i)->move(list.value(i)->pos().x(),list.value(i)->pos().y()+notice_size.height()+1);
        }
    }

    QTimer::singleShot(0,this,SLOT(nextInQueue()));
}

NoticeManager::~NoticeManager()
{
    if(!queue.isEmpty()) queue.clear();

    if(!list.isEmpty())
    {
        foreach(NoticeWindow *wnd, list) wnd->reject();
        list.clear();
    }
}

void NoticeManager::nextInQueue()
{
    if(!queue.isEmpty() && list.size() < maxnotices)
    {
        QPair<QStringList,NoticeWindow::WindowType> newmessage = queue.first();
        queue.removeFirst();
        showNotice(newmessage.first.value(0),newmessage.first.value(1),newmessage.second);
    }
}
