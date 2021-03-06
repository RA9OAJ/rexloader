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

#include "searchline.h"

SearchLine::SearchLine(QWidget *parent) :
    QLineEdit(parent)
{
    actFastSearch = new QAction(tr("Быстрый поиск"),this);
    actFastSearch->setCheckable(true);
    actFastSearch->setChecked(true);
    actShowVariants = new QAction(tr("Показывать варианты"),this);
    actShowVariants->setCheckable(true);
    actShowVariants->setChecked(true);
    actShowAdvancedSearch = new QAction(tr("Расширенный поиск"),this);
    actShowAdvancedSearch->setDisabled(true);
    actRegExp = new QAction(tr("Регулярные выражения"),this);
    actRegExp->setCheckable(true);
    actRegExp->setChecked(true);
    actCaseSensitive = new QAction(tr("С учетом регистра"),this);
    actCaseSensitive->setCheckable(true);
    actCaseSensitive->setChecked(true);
    actClearVariantList = new QAction(tr("Очистить список вариантов"),this);
    popuplist = new PopupList(this);

    btnSearch = new QPushButton(this);
    btnSearch->resize(16,16);
    btnSearch->setCursor(QCursor(Qt::ArrowCursor));
    btnSearch->setStyleSheet("QPushButton {border: none; image: url(:/searchline/find.png);}"
                             "QPushButton::hover {image: url(:/searchline/find1.png);}"
                             "QPushButton::pressed {image: url(:/searchline/find.png);}"
                             "QPushButton::disabled {image: url(:/searchline/find.png);}");

    setTextMargins(btnSearch->width()+2,1,1,1);

    btnClear = new QPushButton(this);
    btnClear->resize(16,16);
    btnClear->setCursor(QCursor(Qt::ArrowCursor));
    btnClear->setStyleSheet("QPushButton {border: none; image: url(:/searchline/clear.png);}"
                             "QPushButton::hover {image: url(:/searchline/clear1.png);}"
                             "QPushButton::pressed {image: url(:/searchline/clear.png);}"
                             "QPushButton::disabled {image: url(:/searchline/clear.png);}");
    btnClear->hide();

    connect(this,SIGNAL(textEdited(QString)),this,SLOT(showClearButton(QString)));
    connect(this,SIGNAL(textChanged(QString)),this,SLOT(search(QString)));
    connect(btnClear,SIGNAL(released()),this,SLOT(clearSearch()));
    connect(actFastSearch,SIGNAL(triggered(bool)),this,SLOT(setOptions(bool)));
    connect(actShowVariants,SIGNAL(triggered(bool)),this,SLOT(setOptions(bool)));
    connect(actRegExp,SIGNAL(triggered(bool)),this,SLOT(setOptions(bool)));
    connect(actCaseSensitive,SIGNAL(triggered(bool)),this,SLOT(setOptions(bool)));
    connect(this,SIGNAL(textChanged(QString)),popuplist,SLOT(setFilter(QString)));
    connect(popuplist,SIGNAL(filterSelected(QString)),this,SLOT(setText(QString)));
    connect(actClearVariantList,SIGNAL(triggered()),popuplist,SLOT(clearVariants()));
}

QByteArray SearchLine::saveSearchState()
{
    QStringList lst = popuplist->getFiltersList();
    QByteArray outbuf;
    QDataStream out(&outbuf,QIODevice::WriteOnly);
    out << actFastSearch->isChecked();
    out << actShowVariants->isChecked();
    out << actRegExp->isChecked();
    out << actCaseSensitive->isChecked();

    out << (qint32) lst.size();
    foreach (QString cur, lst)
    {
        out << (qint32) cur.toAscii().size();
        out.writeRawData(cur.toAscii().data(),cur.toAscii().size());
    }

    return outbuf;
}

bool SearchLine::restoreSearchState(const QByteArray &state)
{
    if(state.isEmpty())
        return false;

    QDataStream in(state);
    bool flag;
    in >> flag;
    actFastSearch->setChecked(flag);
    in >> flag;
    actShowVariants->setChecked(flag);
    in >> flag;
    actRegExp->setChecked(flag);
    in >> flag;
    actCaseSensitive->setChecked(flag);
    \
    qint32 sz;
    in >> sz;
    for(int i = 0; i < sz; ++i)
    {
        qint32 cursz;
        in >> cursz;
        QByteArray inbuf;
        inbuf.resize(cursz);
        in.readRawData(inbuf.data(),cursz);
        popuplist->addVariant(QString(inbuf));
    }
    return true;
}

void SearchLine::setMirror(SearchLine *ln)
{
    _mirror = ln;
}

SearchLine* SearchLine::mirror()
{
    return _mirror.isNull() ? 0 : _mirror.data();
}

void SearchLine::setSourceSortFilterModel(QSortFilterProxyModel *mdl)
{
    _ssfp = mdl;
}

void SearchLine::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        if(!text().isEmpty())
            clearSearch();
        else clearFocus();
        break;
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if(!text().isEmpty())
        {
            search();
            clearFocus();
            popuplist->addVariant(text());
        }
        break;
    default:
        QLineEdit::keyPressEvent(event);
        break;
    }
}

void SearchLine::mouseReleaseEvent(QMouseEvent *event)
{
    QLineEdit::mouseReleaseEvent(event);
}

void SearchLine::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);
}

void SearchLine::resizeEvent(QResizeEvent *event)
{
    QLineEdit::resizeEvent(event);

    QSize btnSz(size().height()-6, size().height()-6);
    btnSearch->setMaximumSize(btnSz);
    btnClear->setMaximumSize(btnSz);
    btnSearch->move(5,(height()-btnSearch->height())/2+1);

    if(btnClear->isVisible())
        setTextMargins(btnSearch->width()+2,1,btnClear->width()+2,1);
    else setTextMargins(btnSearch->width()+2,1,1,1);
}

void SearchLine::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *cmenu = createStandardContextMenu();
    cmenu->addSeparator();
    cmenu->addAction(actFastSearch);
    cmenu->addAction(actShowVariants);
    cmenu->addAction(actRegExp);
    cmenu->addAction(actCaseSensitive);
    cmenu->addAction(actShowAdvancedSearch);
    cmenu->addSeparator();
    cmenu->addAction(actClearVariantList);
    cmenu->exec(event->globalPos());
    delete cmenu;
}

void SearchLine::focusOutEvent(QFocusEvent *event)
{
    if(!popuplist->hasFocus() && popuplist->isVisible())
        popuplist->hide();

    QLineEdit::focusOutEvent(event);
}

void SearchLine::showVariantList(const QString &str)
{
    if(str.isEmpty())
        popuplist->hide();
    else
        popuplist->setFilter(str);
}

void SearchLine::showClearButton(const QString &text)
{
    if(!text.isEmpty())
    {
        setTextMargins(btnSearch->width()+2,1,btnClear->width()-1,1);
        btnClear->move(width()-btnClear->width()-6,(height()-btnSearch->height())/2+1);
        btnClear->show();
        showVariantList(text);
    }
    else
    {
        setTextMargins(btnSearch->width()+2,1,1,1);
        btnClear->hide();
        setFocus();
        popuplist->hide();
    }
}

void SearchLine::setOptions(bool stat)
{
    QAction *sndr = qobject_cast<QAction*>(sender());
    if(sndr == actFastSearch)
        if(stat)
            connect(this,SIGNAL(textChanged(QString)),this,SLOT(search(QString)));
        else
            disconnect(this,SIGNAL(textChanged(QString)),this,SLOT(search(QString)));
    else if(sndr == actCaseSensitive || sndr == actRegExp)
        search(text());
}

QSortFilterProxyModel* SearchLine::sourceSortFilterModel()
{
    return _ssfp.isNull() ? 0 : _ssfp.data();
}

void SearchLine::clearSearch()
{
    clear();
    showClearButton(QString());
    if(_mirror)
        _mirror->clearSearch();

    if(_ssfp && _sfmodels.size() == 2)
    {
        _ssfp->setSourceModel(_sfmodels.first()->sourceModel());
        _ssfp->setFilterRole(_sfmodels.first()->filterRole());
        _ssfp->setFilterKeyColumn(_sfmodels.first()->filterKeyColumn());
        _ssfp->setFilterRegExp(_sfmodels.first()->filterRegExp());
        _ssfp->setSortRole(100);
        foreach (QSortFilterProxyModel *cur, _sfmodels) {
            delete cur;
        }
        _sfmodels.clear();
    }
}

void SearchLine::search()
{
    search(text());
}

void SearchLine::search(const QString &text)
{
    if(_ssfp)
    {
        /*if(_sfmodels.size() != 2)
            _sfmodels.clear();*/

        if(_sfmodels.isEmpty())
        {
            QSortFilterProxyModel *proxy = new QSortFilterProxyModel(this);
            proxy->setSortRole(100);
            proxy->setFilterRole(_ssfp->filterRole());
            proxy->setFilterKeyColumn(_ssfp->filterKeyColumn());
            proxy->setFilterRegExp(_ssfp->filterRegExp());
            proxy->setSourceModel(_ssfp->sourceModel());
            _sfmodels.append(proxy);

            proxy = new QSortFilterProxyModel(this);
            proxy->setSourceModel(_ssfp->sourceModel());
            proxy->setFilterRole(Qt::DisplayRole);
            _ssfp->setSourceModel(proxy);
            _ssfp->setFilterRegExp("");
            _sfmodels.append(proxy);
        }
        _sfmodels.last()->setFilterKeyColumn(1);

        if(actRegExp->isChecked())
        {
            QRegExp reg(text);
            reg.setPatternSyntax(QRegExp::Wildcard);
            _sfmodels.last()->setFilterRegExp(QRegExp(text));
        }
        else _sfmodels.last()->setFilterFixedString(text);
        if(!_sfmodels.last()->rowCount())
            _sfmodels.last()->setFilterKeyColumn(3);

        if(actCaseSensitive->isChecked())_sfmodels.last()->setFilterCaseSensitivity(Qt::CaseSensitive);
        else _sfmodels.last()->setFilterCaseSensitivity(Qt::CaseInsensitive);

        if(!_sfmodels.last()->rowCount())
            setStyleSheet("SearchLine {color: red;}");
        else
            setStyleSheet("");
    }
}
