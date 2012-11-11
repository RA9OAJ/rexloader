/*
Project: REXLoader (Downloader), Source file: floatingwindow.h
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

#ifndef FLOATINGWINDOW_H
#define FLOATINGWINDOW_H

#include <QDialog>
#include <QTimer>
#include <QEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include "progressbar.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <QX11Info>
#endif

class FloatingWindow : public QDialog
{
    Q_OBJECT
public:
    explicit FloatingWindow(QWidget *parent = 0);
    virtual ~FloatingWindow();

public slots:
    void moveToAllDesktops(bool _flag = true);
    
signals:
    
protected:
    virtual bool event (QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

protected slots:
    void fadeAction();
    
private:
    float opacity;
    float minimumOpacity;
    QTimer *optimer;
    QPoint startPos;
    bool moveFlag;
};

#endif // FLOATINGWINDOW_H
