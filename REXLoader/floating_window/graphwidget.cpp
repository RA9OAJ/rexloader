/*
Project: REXLoader (Downloader), Source file: graphwidget.cpp
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

#include "graphwidget.h"

GraphWidget::GraphWidget(QWidget *parent) :
    QWidget(parent)
{
    out_spdf = true;
    rstyle = RS_Graph;
    pub = false;
    schedule_enable = true;
    max = 1024;

    //schedule();
}

GraphWidget::~GraphWidget()
{
    schedule_enable = false;
}

void GraphWidget::addPoint(qint64 val)
{
    points.prepend(val);
    if(points.size() > size().width() && points.value(size().width()-1) == max)
    {
        //points.removeAt(size().width()-1);
        max = 1024;
        for(int i = 0; i < size().width()-1; ++i)
           if(max < points.value(i))
           max = points.value(i);
    }

    if(points.size() > 255)
        points.removeLast();

    if(val > max)
        max = val;

    pub = false;
    repaint();
}

void GraphWidget::setRenderStyle(int style)
{
    rstyle = (RenderStyle)style;
}

void GraphWidget::setSpeedFormat(bool out_bytes)
{
    out_spdf = out_bytes;
}

void GraphWidget::paintEvent(QPaintEvent *e)
{
    QPainter *p = new QPainter(this);
    p->fillRect(QRect(0,0,size().width(),size().height()),QColor("#bfdbe0"));

    switch(rstyle)
    {
    case RS_Diagram:
        renderDiagram(p);
        break;
    default:
        renderGraph(p);
    }

    QFont font;
    font.setPixelSize(10);
    p->setFont(font);

    QStringList out = speedForHumans((points.isEmpty() ? 0 : points.first()),true,out_spdf);
    p->drawText(QRect(3,2,100,12),QString("%1%2").arg(out.first(),out.last()));
    p->end();

    e->accept();
}

void GraphWidget::renderGraph(QPainter *p)
{
    p->save();
    p->setPen(QColor("#2ac2de"));
    int y = 0;

    if(points.size() == 0)
    {
        int height = (size().height()-3)*points.value(0)/max;
        p->drawPoint(size().width()-1,size().height()-1-height);
    }
    else
        for(int i = size().width()-1; i > 0 && y < points.size()-1; --i, ++y)
        {
            int height1 = (size().height()-3)*points.value(y)/max;
            int height2 = (size().height()-3)*points.value(y+1)/max;
            p->drawLine(i,size().height()-height1,i-1,size().height()-height2);
        }

    p->restore();
}

void GraphWidget::renderDiagram(QPainter *p)
{
    p->save();
    p->setPen(QColor("#2ac2de"));
    int y = 0;
    for(int i = size().width()-1; i >= 0 && y < points.size(); --i, ++y)
    {
        int height = (size().height()-3)*points.value(y)/max;
        p->drawLine(i,size().height()-1-height,i,size().height()-1);
    }

    p->restore();
}

QStringList GraphWidget::speedForHumans(qint64 sp, bool in_bytes, bool out_bytes)
{
    QStringList outstrings;
    if(in_bytes && !out_bytes)sp = sp*8;
    if(!in_bytes && out_bytes)sp = sp/8;
    if(sp >= 1073741824)outstrings << QString::number((qint64)sp/1073741824.0,'f',1) << (out_bytes ? tr(" ГБ/с"):tr(" Гб/с"));
    else if(sp >= 1048576)outstrings << QString::number((qint64)sp/1048576.0,'f',1) << (out_bytes ? tr(" МБ/с"):tr(" Мб/с"));
    else if(sp >= 1024)outstrings << QString::number((qint64)sp/1024.0,'f',1) << (out_bytes ? tr(" кБ/с"):tr(" кб/с"));
    else outstrings << QString::number(sp) << (out_bytes ? tr(" Байт/с"):tr(" бит/с"));

    return outstrings;
}

void GraphWidget::schedule()
{
    /*if(!pub)
        addPoint(0);*/

    repaint();

    if(schedule_enable)
        QTimer::singleShot(1000,this,SLOT(schedule()));
}
