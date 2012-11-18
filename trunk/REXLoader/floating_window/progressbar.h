/*
Project: REXLoader (Downloader), Source file: progressbar.h
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

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QDebug>

class ProgressBar : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressBar(QWidget *parent = 0);
    virtual ~ProgressBar();
    
    int maximumValue() const;
    int value() const;
    void setToolTipFormat(const QString &text);

signals:
    
public slots:
    void setMaxValue(int max);
    void setValue(int val);

protected:
    virtual void paintEvent(QPaintEvent *e);

private:
    int maxval;
    int _value;
    float arg;
    QString toolout;

};

#endif // PROGRESSBAR_H
