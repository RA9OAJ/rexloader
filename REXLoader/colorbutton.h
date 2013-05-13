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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QPushButton>
#include <QColorDialog>
#include <QPainter>

class ColorButton : public QPushButton
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget *parent = 0);
    ~ColorButton();

    void setText(const QString &text);
    void setMenu(QMenu *menu);
    void setDefaultColor(const QColor &color);
    QColor currentColor() const;

signals:
    void colorSelected(const QColor &color);

public slots:
    void setColor(const QColor &color);
    void resetToDefault();

protected:
    virtual void paintEvent(QPaintEvent *event);

protected slots:
    void showColorDialog();
    void cancelColorDialog();

private:
    QColorDialog *dlg;
    QColor def_color;
    QColor cur_color;

    QByteArray colordlg_stat;
};

#endif // COLORBUTTON_H
