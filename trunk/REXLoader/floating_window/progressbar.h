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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QToolTip>
#include <QDebug>

class ProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressBar(QWidget *parent = 0);
    virtual ~ProgressBar();
    
    int maximumValue() const;
    int value() const;
    void setMyToolTip(const QString &text);
    QString myToolTip() const;

signals:
    void doubleClick();

public slots:
    void setMaxValue(int max);
    void setValue(int val);

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void enterEvent(QEvent *e);

protected slots:
    void showToolTip();

private:
    int maxval;
    int _value;
    float arg;
    QString tooltip_text;
};

#endif // PROGRESSBAR_H
