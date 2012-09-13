/*
Project: REXLoader (Downloader), Source file: pluginitemdelegate.h
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

#ifndef PLUGINITEMDELEGATE_H
#define PLUGINITEMDELEGATE_H

#include <QStyledItemDelegate>
#include <QApplication>
#include <QPainter>
#include <QFont>
#include <QStyleOptionViewItem>

class PluginItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit PluginItemDelegate(QObject *parent = 0);
    ~PluginItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;
    
signals:
    
public slots:

protected:
    void paintBody(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paintGrid(QPainter *painter, const QStyleOptionViewItem &option) const;
    void resetOptions();

private:
    QFont bigFont, bigBoldFont, normFont,boldFont, smallFont, smallBoldFont;
    QString protoText, pluginText, authorText, licText;
    int dx1,dx2,dx3_1,dx3_2;
};

#endif // PLUGINITEMDELEGATE_H
