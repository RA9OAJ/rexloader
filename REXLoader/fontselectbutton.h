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

#ifndef FONTSELECTBUTTON_H
#define FONTSELECTBUTTON_H

#include <QPushButton>
#include <QPainter>
#include <QFontDialog>
#include <QApplication>

class FontSelectButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FontSelectButton(QWidget *parent = 0);
    ~FontSelectButton();

    enum AutoResizeFlag{
        NoResize = 0,
        OnWidth = 1,
        OnHeight = 2,
        FullResize = OnWidth | OnHeight
    };

    QFont font() const;
    QColor fontColor() const;
    QColor backgroundColor() const;
    void setDefaultFont(const QFont &fnt);
    void setDefaultFontColor(const QColor &color);
    void setText(const QString &text);
    void setAutoResize(AutoResizeFlag flag);
    int autoResizeFlag() const;

signals:
    void fontSelected(const QFont &fnt);

public slots:
    void setFont(const QFont &fnt);
    void setFontColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    void resetToDefault();

protected:
    virtual void paintEvent(QPaintEvent *event);

protected slots:
    void showFontDialog();
    void cancelFontDialog();

private:
    QString _sample_string;
    AutoResizeFlag _flg;
    QFont _fnt;
    QFont _def_fnt;
    QColor _fnt_color;
    QColor _def_color;
    QColor _back_color;
    QColor _def_back_color;
    QByteArray _dlg_stat;

    QFontDialog *_dlg;

};

#endif // FONTSELECTBUTTON_H
