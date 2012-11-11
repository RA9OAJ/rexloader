/*
Project: REXLoader (Downloader), Source file: floatingwindow.cpp
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

#include "floatingwindow.h"

FloatingWindow::FloatingWindow(QWidget *parent) :
    QDialog(parent)
{
    setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    moveToAllDesktops();
    moveFlag = false;

    optimer = new QTimer(this);
    optimer->setInterval(13);
    connect(optimer,SIGNAL(timeout()),this,SLOT(fadeAction()));

    opacity = 1.0;
    minimumOpacity = 0.35;
    setWindowOpacity(minimumOpacity);
    setLayout(new QVBoxLayout(this));
    layout()->setMargin(3);
    resize(150,75);
}

FloatingWindow::~FloatingWindow()
{
}

void FloatingWindow::moveToAllDesktops(bool _flag)
{
#ifdef Q_WS_X11
    Atom atom = XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", false);
    if (atom)
    {
        unsigned int val;
        if(_flag) val = 0xffffffff;
        else val = 0x2;
        XChangeProperty(QX11Info::display(), this->winId(), atom, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&val, 1);
    }
#endif
}

bool FloatingWindow::event(QEvent *event)
{
    if(event->type() == QEvent::Enter || event->type() == QEvent::Leave)
        optimer->start();

    return QDialog::event(event);
}

void FloatingWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton)
    {
        startPos = event->globalPos();
        moveFlag = true;
    }
    else QDialog::mousePressEvent(event);
}

void FloatingWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->buttons() == Qt::LeftButton && moveFlag)
        moveFlag = false;
    else QDialog::mouseReleaseEvent(event);
}

void FloatingWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(moveFlag)
    {
        QPoint diff = event->globalPos() - startPos;
        startPos = event->globalPos();
        move(pos() + diff);
    }
    else QDialog::mouseMoveEvent(event);
}

void FloatingWindow::fadeAction()
{
    if(underMouse())
        opacity += 0.05;
    else opacity -= 0.05;

    if(opacity <= minimumOpacity || opacity >= 1.0)
    {
        optimer->stop();
        opacity = opacity >= 1.0 ? 1.0 : minimumOpacity;
    }

    setWindowOpacity(opacity);
}
