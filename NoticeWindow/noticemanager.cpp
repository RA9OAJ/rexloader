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
    notices.clear();
    notice_cnt = 0;
    maxnotices = 5;
    all_desktop = true;
    notice_size = QSize(200,100);
    dif = QSize(3,3);
    effect = NoticeWindow::SE_PopUp;
}

void NoticeManager::showNotice(const QString &title, const QString &message, NoticeWindow::WindowType type)
{
    NoticeWindow *wnd = new NoticeWindow();
    wnd->setAttribute(Qt::WA_DeleteOnClose);
    wnd->setMaximumSize(notice_size);
    list.append(wnd);
    wnd->setDisplayTime(10000);
    connect(wnd,SIGNAL(rejected()),this,SLOT(closeOneWindow()));
    if(notice_cnt <= maxnotices)
    {
        if(!notice_cnt) wnd->setOffsetPos(dif.width(),dif.height());
        else
        {
            wnd->setDOffsetPos(0,notice_cnt*notice_size.height()+dif.height()+notice_cnt-1);
            wnd->setOffsetPos(dif.width(),1);
        }
        wnd->showNotice(title,message,type);
    }
    notice_cnt++;
}

void NoticeManager::closeOneWindow()
{
    NoticeWindow *wnd = qobject_cast<NoticeWindow*>(sender());
    if(!wnd)return;
    int id = list.indexOf(wnd);
    if(id < 0)return;

    if(id != list.size()-1 || list.size() > 1)
    {
        for(int i = id+1; i < list.size(); i++)
            list.value(i)->move(list.value(i)->pos().x(),list.value(i)->pos().y()+notice_size.height()+1);
    }
    list.removeOne(wnd);
    notice_cnt--;
}
