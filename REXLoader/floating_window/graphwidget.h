/*
Project: REXLoader (Downloader), Source file: graphwidget.h
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

#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QPaintEvent>
#include <QDebug>

class GraphWidget : public QWidget
{
    Q_OBJECT
public:
    enum RenderStyle{
        RS_Graph,
        RS_Diagram
    };

    explicit GraphWidget(QWidget *parent = 0);
    ~GraphWidget();
    
signals:
    
public slots:
    void addPoint(qint64 val);
    void setRenderStyle(int style);
    void setSpeedFormat(bool out_bytes);

protected:
    virtual void paintEvent(QPaintEvent *e);
    void renderGraph(QPainter *p);
    void renderDiagram(QPainter *p);
    QStringList speedForHumans(qint64 sp, bool in_bytes = true, bool out_bytes = false);

protected slots:
    void schedule();

private:
    QList<qint64> points;
    RenderStyle rstyle;
    qint64 max;
    bool pub;
    bool schedule_enable;
    bool out_spdf;
};

#endif // GRAPHWIDGET_H
