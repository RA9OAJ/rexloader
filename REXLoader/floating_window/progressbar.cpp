/*
Copyright (C) 2012-2013  Sarvaritdinov R.

This file is part of REXLoader.

REXLoader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

REXLoader is distributed in the hope that it will be useful,
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
    setMinimumHeight(5);
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

void ProgressBar::setMyToolTip(const QString &text)
{
    tooltip_text = text;

    if(QToolTip::isVisible())
        showToolTip();
}

QString ProgressBar::myToolTip() const
{
    return tooltip_text;
}

void ProgressBar::setMaxValue(int max)
{
    maxval = max;
    arg = (float)_value/(float)maxval;
    if(arg > 1.0)
        arg = 1.0;

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
    p->setPen(QColor("#abc6cb")); //abc6cb b4d0d5 006e8e
    p->drawRect(QRect(0,0,size().width()-1,size().height()-1));
    p->fillRect(QRectF(arg*(size().width()-2)+1,1,size().width()-arg*(size().width()-2)-2,size().height()-2),QColor("#b9d3dc"));
    p->fillRect(QRectF(1,1,arg*(size().width()-2),size().height()-2),QColor("#2ac2de")); //0191ba
    p->end();

    e->accept();
}

void ProgressBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    QWidget::mouseDoubleClickEvent(e);
    emit doubleClick();
}

void ProgressBar::enterEvent(QEvent *e)
{
    QWidget::enterEvent(e);

    QTimer::singleShot(150,this,SLOT(showToolTip()));
}

void ProgressBar::showToolTip()
{
    if(underMouse())
        QToolTip::showText(QCursor::pos(),tooltip_text,this);
}
