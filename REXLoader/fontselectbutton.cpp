/*
Project: REXLoader (Downloader), Source file: selectfontbutton.cpp
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

#include "fontselectbutton.h"

FontSelectButton::FontSelectButton(QWidget *parent) :
    QPushButton(parent)
{
    _dlg = 0;
    _fnt_color = _def_color = QApplication::palette().color(QPalette::WindowText);
    _fnt = _def_fnt = QApplication::font();
    connect(this,SIGNAL(released()),this,SLOT(showFontDialog()));
}

QFont FontSelectButton::font() const
{
    return _fnt;
}

QColor FontSelectButton::fontColor() const
{
    return _fnt_color;
}

void FontSelectButton::setDefaultFont(const QFont &fnt)
{
    _def_fnt = fnt;
}

void FontSelectButton::setDefaultFontColor(const QColor &color)
{
    _def_color = color;
}

void FontSelectButton::setFont(const QFont &fnt)
{
    _fnt = fnt;
    if(_dlg == qobject_cast<QFontDialog*>(sender()))
    {
        _dlg_stat = _dlg->saveGeometry();
        _dlg->deleteLater();
        _dlg = 0;
    }
    emit fontSelected(_fnt);
}

void FontSelectButton::setFontColot(const QColor &color)
{
}

void FontSelectButton::resetToDefault()
{
}

void FontSelectButton::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);
}

void FontSelectButton::showFontDialog()
{
    if(_dlg)
    {
        _dlg->activateWindow();
        return;
    }

    _dlg = new QFontDialog(this);
    if(!_dlg_stat.isEmpty()) _dlg->restoreGeometry(_dlg_stat);
    _dlg->setCurrentFont(_fnt);
    _dlg->setOption(QFontDialog::DontUseNativeDialog);
    _dlg->setModal(false);
    connect(_dlg,SIGNAL(fontSelected(QFont)),this,SLOT(setFont(QFont)));
    connect(_dlg,SIGNAL(rejected()),this,SLOT(cancelFontDialog()));
    _dlg->show();
}

void FontSelectButton::cancelFontDialog()
{
    if(_dlg == qobject_cast<QFontDialog*>(sender()))
    {
        _dlg_stat = _dlg->saveGeometry();
        _dlg->deleteLater();
        _dlg = 0;
    }
}

FontSelectButton::~FontSelectButton()
{
    if(_dlg) _dlg->deleteLater();
}
