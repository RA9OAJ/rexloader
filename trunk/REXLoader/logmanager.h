/*
Project: REXLoader (Downloader), Source file: logmanager.h
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

#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QList>
#include <QBoxLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QPointer>
#include <QTableWidget>
#include <QTimer>
#include "logtreemodel.h"

class LogManager : public QObject
{
    Q_OBJECT
public:
    explicit LogManager(QObject *parent = 0);
    virtual ~LogManager();

    LogTreeModel* model(int table_id = -1, int id_sect = 0) const; //возвращает модель лога
    void setTabWidget(QTabWidget *widget); //устанавливает для управления виджет вкладок

public slots:
    void appendLog(int table_id, int id_sect, int mtype, const QString &title, const QString &more); //добавляет запись в лог задания по table_id
    void setMaxStringCount(int max); //устанавливает максимальное количество строк в логах
    void saveLogToFile(const QString &filename, int table_id = -1); //сохраняет лог для задания table_id в файл, если table_id == -1,то выгружаются все логи
    void loadLogFromFile(const QString &file); //загружает лог из файла
    void clearLog(int table_id = -1); //очищает лог для задания table_id, если table_id == -1, то очищаются все логи
    void manageTabs(int table_id); //метод управляет добавлением вкладок
    void setDeleteInterval(int sec); //устанавливает интервал до удаления после неактивного поведения лога секции
    void deleteTaskLog(int table_id); //немедленно удаляет логи секций для задания table_id
    void deleteTaskLogLater(int table_id); //удаляет логи секций для задания table_id через интервал, установленный в setDeleteInterval()
    void setLogColor(int m_type, const QColor &color);
    void setLogFont(int m_type, const QFont &font);
    void setLogFontColor(int m_type, const QColor &color);

protected:
    QWidget* createTabWidget();
    QTreeView* getTreeView(QWidget *wgt);

protected slots:
    void timerManager();

private:
    QHash<int, QList<LogTreeModel*> > loglist; //хэш списков моделей по table_id
    QPointer<QTabWidget> _tabwidget;
    int _max_str_count; //максимальное количество строк в логах
    int _cur_table_id;
    int _del_interval; //интервал до удаления логов по таймеру

    QHash<int,QTimer*> timers;

    QHash<int, QColor> row_color;
    QHash<int, QFont> fonts;
    QHash<int, QColor> font_color;
};

#endif // LOGMANAGER_H
