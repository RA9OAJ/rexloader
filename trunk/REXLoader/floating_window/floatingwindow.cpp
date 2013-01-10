/*
Project: REXLoader (Downloader), Source file: floatingwindow.cpp
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

#include "floatingwindow.h"

FloatingWindow::FloatingWindow(QWidget *parent) :
    QDialog(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    setContextMenuPolicy(Qt::CustomContextMenu);
    setWindowTitle("");
    moveToAllDesktops();
    moveFlag = false;
    renderstyle = GraphWidget::RS_Graph;

    menu = new QMenu(this);
    QAction* act = new QAction(tr("Диаграмма"),menu);
    act->setCheckable(true);
    act->setChecked(false);
    act->setObjectName("Diagramm");
    connect(act,SIGNAL(triggered(bool)),SLOT(setRenderMode(bool)));
    menu->addAction(act);
    act = new QAction(tr("График"),menu);
    act->setCheckable(true);
    act->setChecked(true);
    act->setObjectName("Graphic");
    connect(act,SIGNAL(triggered(bool)),SLOT(setRenderMode(bool)));
    menu->addAction(act);
    menu->addSeparator();
    QMenu *submnu = new QMenu(tr("Отображать"),menu);
    submnu->setObjectName("SubMenu");
    menu->addMenu(submnu);
    act = new QAction(tr("Всегда"),submnu);
    act->setCheckable(true);
    act->setChecked(true);
    connect(act,SIGNAL(triggered(bool)),SLOT(setShowMode(bool)));
    act->setObjectName("ShowAlways");
    submnu->addAction(act);
    act = new QAction(tr("Во время закачки"),submnu);
    act->setCheckable(true);
    act->setChecked(false);
    act->setObjectName("ShowDownloadOnly");
    submnu->addAction(act);
    connect(act,SIGNAL(triggered(bool)),SLOT(setShowMode(bool)));
    menu->addSeparator();
    act = new QAction(tr("Скрыть"),menu);
    connect(act,SIGNAL(triggered()),this,SLOT(hide()));
    menu->addAction(act);

    optimer = new QTimer(this);
    optimer->setInterval(13);
    connect(optimer,SIGNAL(timeout()),this,SLOT(fadeAction()));
    connect(this,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showContextMenu(QPoint)));

    maximumOpacity = opacity = 0.9;
    minimumOpacity = 0.45;
    setWindowOpacity(minimumOpacity);
    setLayout(new QVBoxLayout(this));
    layout()->setMargin(3);
    layout()->setSpacing(1);
    resize(125,65);

    graph = new GraphWidget(this);
    graph->setMinimumHeight(50);
    layout()->addWidget(graph);
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

void FloatingWindow::currentSpeed(qint64 spd)
{
    graph->addPoint(spd);
}

void FloatingWindow::startTask(int id)
{
    if(tasksbars.contains(id))
        return;

    ProgressBar *bar = new ProgressBar(this);
    bar->setMaximumHeight(5);
    bar->setMaxValue(100);
    bar->setValue(0);
    bar->setToolTipFormat(tr("Выполнено на %v"));
    tasksbars.insert(id,bar);
    layout()->addWidget(bar);

    if((menu->findChild<QAction*>("ShowDownloadOnly"))->isChecked() && isHidden())
        show();
}

void FloatingWindow::stopTask(int id)
{
    if(!tasksbars.contains(id))
        return;

    ProgressBar *bar = tasksbars.value(id);
    layout()->removeWidget(bar);
    bar->hide();
    resize(size().width(),size().height()-6);
    bar->deleteLater();
    tasksbars.remove(id);

    if((menu->findChild<QAction*>("ShowDownloadOnly"))->isChecked() && !isHidden() && !tasksbars.count())
        hide();
}

void FloatingWindow::taskData(int id, qint64 total, qint64 load)
{
    if(!tasksbars.contains(id))
        return;

    if(!total)
        return;

    ProgressBar *bar = tasksbars.value(id);
    bar->setMaxValue(100);
    int cur = load*100/total;
    bar->setValue(cur);
}

void FloatingWindow::show()
{
    if(_disable)
        return;

    if(((menu->findChild<QAction*>("ShowDownloadOnly"))->isChecked() && tasksbars.count()) || (menu->findChild<QAction*>("ShowAlways"))->isChecked())
        QDialog::show();

}

void FloatingWindow::disableWindow(bool dis)
{
    _disable = dis;
}

void FloatingWindow::setRenderGraphMode(int md)
{
    if(md == GraphWidget::RS_Diagram)
    {
        (menu->findChild<QAction*>("Graphic"))->setChecked(false);
        (menu->findChild<QAction*>("Diagramm"))->setChecked(true);
        graph->setRenderStyle(GraphWidget::RS_Diagram);
        renderstyle = GraphWidget::RS_Diagram;
    }
    else
    {
        (menu->findChild<QAction*>("Diagramm"))->setChecked(false);
        (menu->findChild<QAction*>("Graphic"))->setChecked(true);
        graph->setRenderStyle(GraphWidget::RS_Graph);
        renderstyle = GraphWidget::RS_Graph;
    }
}

void FloatingWindow::setSpeedFormat(bool bytes)
{
    graph->setSpeedFormat(bytes);
}

void FloatingWindow::setShowWindowMode(bool md)
{
    if(md)
    {
        (menu->findChild<QAction*>("ShowDownloadOnly"))->setChecked(false);
        (menu->findChild<QAction*>("ShowAlways"))->setChecked(true);
        show_always = true;
    }
    else
    {
        show_always = false;
        (menu->findChild<QAction*>("ShowAlways"))->setChecked(false);
        (menu->findChild<QAction*>("ShowDownloadOnly"))->setChecked(true);
        if(!tasksbars.count())
            hide();
    }
}

int FloatingWindow::renderGraphMode()
{
    return renderstyle;
}

bool FloatingWindow::showWindowsMode()
{
    return show_always;
}

QMenu *FloatingWindow::subMenu()
{
    return menu->findChild<QMenu*>("SubMenu");
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

    if(opacity <= minimumOpacity || opacity >= maximumOpacity)
    {
        optimer->stop();
        opacity = opacity >= maximumOpacity ? maximumOpacity : minimumOpacity;
    }

    setWindowOpacity(opacity);
}

void FloatingWindow::showContextMenu(const QPoint &pos)
{
    menu->popup(mapToGlobal(pos));
}

void FloatingWindow::setRenderMode(bool checked)
{
    QAction *act = qobject_cast<QAction*>(sender());
    if(!act)
        return;
    if(!checked)
        act->setChecked(true);

    if(act == menu->findChild<QAction*>("Diagramm"))
    {
        act->setChecked(true);
        (menu->findChild<QAction*>("Graphic"))->setChecked(false);
        graph->setRenderStyle(GraphWidget::RS_Diagram);
        renderstyle = GraphWidget::RS_Diagram;
    }
    else
    {
        act->setChecked(true);
        (menu->findChild<QAction*>("Diagramm"))->setChecked(false);
        graph->setRenderStyle(GraphWidget::RS_Graph);
        renderstyle = GraphWidget::RS_Graph;
    }
}

void FloatingWindow::setShowMode(bool checked)
{
    QAction *act = qobject_cast<QAction*>(sender());
    if(!act)
        return;
    if(!checked)
        act->setChecked(true);

    if(act == menu->findChild<QAction*>("ShowAlways"))
    {
        (menu->findChild<QAction*>("ShowDownloadOnly"))->setChecked(false);
        show_always = true;
        show();
    }
    else
    {
        show_always = false;
        (menu->findChild<QAction*>("ShowAlways"))->setChecked(false);
        if(!tasksbars.count())
            hide();
    }
}
