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

#ifndef NOTICEMANAGER_H
#define NOTICEMANAGER_H

#include <QObject>
#include "noticewindow.h"

class NoticeManager : public QObject
{
    Q_OBJECT

public:
    NoticeManager(QObject *parent = 0);

public slots:
    void showNotice(const QString &title, const QString &message, NoticeWindow::WindowType type = NoticeWindow::WT_Info);
    void closeOneWindow();

private:
    QList<QStringList> notices;
    QList<NoticeWindow*> list;
    int notice_cnt;
    int maxnotices;
    QSize notice_size;
    QSize dif;
    bool all_desktop;
    int effect;

};

#endif // NOTICEMANAGER_H
