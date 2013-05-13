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

#ifndef SEARCHLINE_H
#define SEARCHLINE_H

#include <QLineEdit>
#include <QPushButton>
#include <QKeyEvent>
#include <QPaintEvent>
#include <QPointer>
#include <QMenu>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

class SearchLine : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchLine(QWidget *parent = 0);
    QByteArray saveSearchState();
    bool restoreSearchState(QByteArray state);
    void setMirror(SearchLine *ln);
    SearchLine* mirror();
    void setSourceSortFilterModel(QSortFilterProxyModel *mdl);
    QSortFilterProxyModel* sourceSortFilterModel();
    
signals:
    
public slots:
    void clearSearch();
    void search();
    void search(const QString &text);

protected:
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void mouseReleaseEvent (QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

protected slots:
    void showVariantList(const QString &str);
    void showClearButton(const QString &text);
    void setOptions(bool stat);

private:
    QPointer<SearchLine> _mirror;
    QPointer<QSortFilterProxyModel> _ssfp;

    QPushButton *btnSearch;
    QPushButton *btnClear;

    QAction *actFastSearch;
    QAction *actShowVariants;
    QAction *actShowAdvancedSearch;
    QAction *actRegExp;
    QAction *actCaseSensitive;

    QList<QSortFilterProxyModel*> _sfmodels;
};

#endif // SEARCHLINE_H
