/*
Project: REXLoader (Downloader), Source file: colorbutton.cpp
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

#include "colorbutton.h"

ColorButton::ColorButton(QWidget *parent) :
    QPushButton(parent)
{
    QPushButton::setText("");
    dlg = 0;
    cur_color = def_color = QColor("#ff0000");
    connect(this,SIGNAL(released()),this,SLOT(showColorDialog()));
}

ColorButton::~ColorButton()
{
    if(dlg) dlg->deleteLater();
}

void ColorButton::setText(const QString &text)
{
    return;
}

void ColorButton::setMenu(QMenu *menu)
{
    return;
}

void ColorButton::setDefaultColor(const QColor &color)
{
    def_color = color;
}

void ColorButton::setColor(const QColor &color)
{
    cur_color = color;
    if(dlg == qobject_cast<QColorDialog*>(sender()))
    {
        colordlg_stat = dlg->saveGeometry();
        dlg->deleteLater();
        dlg = 0;
    }
    emit colorSelected(color);
}

QColor ColorButton::currentColor() const
{
    return cur_color;
}

void ColorButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.translate(0,0);

    int x, y, wdth, hght;
    x = size().width()/4;
    y = size().height()/4;
    wdth = size().width()/2;
    hght = size().height()/2;

    painter.fillRect(x,y,wdth,hght,cur_color);
    painter.end();
}

void ColorButton::showColorDialog()
{
    if(dlg)
    {
        dlg->activateWindow();
        return;
    }

    dlg = new QColorDialog(this);
    if(!colordlg_stat.isEmpty()) dlg->restoreGeometry(colordlg_stat);
    dlg->setCurrentColor(cur_color);
    dlg->setOption(QColorDialog::DontUseNativeDialog);
    dlg->setModal(false);
    connect(dlg,SIGNAL(colorSelected(QColor)),this,SLOT(setColor(QColor)));
    connect(dlg,SIGNAL(rejected()),this,SLOT(cancelColorDialog()));
    dlg->show();
}

void ColorButton::cancelColorDialog()
{
    if(dlg == qobject_cast<QColorDialog*>(sender()))
    {
        colordlg_stat = dlg->saveGeometry();
        dlg->deleteLater();
        dlg = 0;
    }
}

void ColorButton::resetToDefault()
{
    cur_color = def_color;
}
