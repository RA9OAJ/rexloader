/*
Project: REXLoader (Downloader), Source file: floatingwindow.h
Copyright (C) 2012  Sarvaritdinov R.

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
#include <QHash>
#include <QMenu>
#include <QAction>

#include "graphwidget.h"
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
    void currentSpeed(qint64 spd);
    void startTask(int id);
    void stopTask(int id);
    void taskData(int id, qint64 total, qint64 load);
    void show();
    void disableWindow(bool dis);
    void setRenderGraphMode(int md);
    void setSpeedFormat(bool bytes);
    void setShowWindowMode(bool md);
    int renderGraphMode();
    bool showWindowsMode();
    QMenu* subMenu();
    
signals:
    
protected:
    virtual bool event (QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

protected slots:
    void fadeAction();
    void showContextMenu(const QPoint &pos);
    void setRenderMode(bool checked);
    void setShowMode(bool checked);
    
private:
    float opacity;
    float minimumOpacity;
    float maximumOpacity;
    QTimer *optimer;
    QPoint startPos;
    bool moveFlag;
    bool _disable;
    bool show_always;

    GraphWidget *graph;
    GraphWidget::RenderStyle renderstyle;
    QHash<int, ProgressBar*> tasksbars;
    QMenu *menu;
};

#endif // FLOATINGWINDOW_H
