/*
Project: REXLoader (Downloader), Source file: progressbar.cpp
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

#include "progressbar.h"

ProgressBar::ProgressBar(QWidget *parent) :
    QWidget(parent)
{
    maxval = 100;
    setValue(0);
}

ProgressBar::~ProgressBar()
{
}

int ProgressBar::maximumValue() const
{
    return maxval;
}

int ProgressBar::value() const
{
    return _value;
}

void ProgressBar::setToolTipFormat(const QString &text)
{
    toolout = text;
}

void ProgressBar::setMaxValue(int max)
{
    maxval = max;
    arg = (float)_value/(float)maxval;
    if(arg > 1.0)
        arg = 1.0;

    QString out = toolout.replace("%v",QString("%1%").arg(_value));
    setToolTip(out);
    repaint();
}

void ProgressBar::setValue(int val)
{
    _value = val;
    arg = (float)val/(float)maxval;
    if(arg > 1.0)
        arg = 1.0;
    repaint();
}

void ProgressBar::paintEvent(QPaintEvent *e)
{
    QPainter *p = new QPainter(this);
    p->setPen(QColor("#006e8e"));
    p->drawRect(QRect(0,0,size().width()-1,size().height()-1));
    p->fillRect(QRectF(arg*(size().width()-2)+1,1,size().width()-arg*(size().width()-2)-2,size().height()-2),QColor("#b9d3dc"));
    p->fillRect(QRectF(1,1,arg*(size().width()-2),size().height()-2),QColor("#0191ba"));
    p->end();

    e->accept();
}
