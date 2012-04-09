/*
Project: REXLoader (Downloader), Source file: REXWindow.h
Copyright (C) 2011  Sarvaritdinov R.

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

#ifndef REXWINDOW_H
#define REXWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QtSql/QtSql>
#include <QDir>
#include <QStringList>
#include <QHash>
#include <QList>
#include <QPluginLoader>
#include <QSortFilterProxyModel>
#include <QtGui>
#include "titemmodel.h"
#include "addtaskdialog.h"
#include "emessagebox.h"
#include "treeitemmodel.h"
#include "settingsdialog.h"
#include "importdialog.h"
#include "categorydialog.h"
#include "taskdialog.h"
#include "pluginmanager.h"
#include "tableview.h"

#include "../Httploader/LoaderInterface.h"

namespace Ui {
    class REXWindow;
}

class REXWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit REXWindow(QWidget *parent = 0);
    virtual ~REXWindow();

    void showNotice(const QString &title, const QString &text, int type = 0);

public slots:
    void updateTaskSheet(); //обновляет содержимое таблицы списка заданий
    void startTrayIconAnimaion(); //запускает анимацию значка в трее
    void stopTrayIconAnimation(); //останавливает анимацию значка трее
    void scanNewTaskQueue(); //сканирует очередь url для добавления в очередь заданий
    void scanClipboard(); //сканирует буфер обмена на наличие доступного для скачивания URL
    void showImportFileDialog(); //отображает диалог для выбора файла импорта

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *e);
    bool event(QEvent *event);
    void openDataBase(); //создает/открывает существующую базу данных закачек
    void saveSettings(); //сохраняет свойства приложения
    void loadSettings(); //загружает свйоства приложения
    void loadPlugins(); //сканирует директории в поисках доступных плагинов, возвращает количество найденных и загруженных плагинов
    void calculateSpeed(); //пересчитывает и перераспределяет скорость между плагинами
    void scanTasksOnStart(); //сканирует список задач при каждом запуске программы, выделяет приостановленные и предлагает их докачать

protected slots:
    void prepareToQuit(); //подгатавливает приложение к завершению (останавливает все закачки, сохраняет настройки приложения и т.д.)
    void pluginStatus(bool stat);
    void scheduler(); //внутренний планировщик с интервалом в 1 секунду
    void updateTrayIcon(); //обновляет иконку в трее при анимации
    void updateStatusBar(); //обновляет данные, отображаемые в statusBar
    void showAddTaskDialog(); //отображает окно добавления задачи
    void showHideSlot(QSystemTrayIcon::ActivationReason type); //отображает/скрывает главное окно
    void deleteTask(); //удаляет выбранные задания из списка задач
    void startTask(); //запускает выбранные задания на выполнение
    void startTask(int id); //запускает задание по id в БД
    void startAllTasks(); //запускает все доступные для запуска задачи
    void stopTask(); //останавливает выполнение выбранной задачи
    void stopTask(int id); //останавливает выполнение задания по id в БД
    void stopAllTasks(); //останавливает все выполняемые задачи
    void redownloadTask(); //сбрасывает признаки скачивания файла и ставит его снова на закачку
    void syncTaskData(); //синхронизирует состояние задач с БД
    void manageTaskQueue(); //менеджер очереди заданий
    void startTaskNumber(int id_row, const QUrl &url, const QString &filename = QString(), qint64 totalload = 0);
    void setProxy(int id_task, int id_proto, bool global = true); //настраивает задание на скачивание через прокси
    void showTableContextMenu(const QPoint &pos); //отображает контекстное меню в таблице
    void openTask(); //отображает окно свойств задачи, если оно не закачано, или же пытается открыть файл стандартными программами
    void openTaskDir(); //открывает в файловом менеджере папку куда сохраняется/сохранен целевой файл
    void setTaskPriority(); //устанавливает выбранный приоритет всем выделенным задачам
    void setTaskFilter(const QModelIndex &index); //устанавливает фильтр на выводимые в таблице задачи в зависимости от выделенной категории
    void importUrlFromFile(const QStringList &files); //импортирует URL из указанного файла
    void acceptQAction(QAbstractButton *btn); //обработчик решений пользователя на задаваемые программой вопросы
    void readSettings(); //считывает параметры настрое после их изменения
    void selectSpeedRate(bool checked); //переключение скорости скачивания
    void showTreeContextMenu(const QPoint &pos); //отображает контекстное меню для дерева категорий и фильтров
    void deleteCategory(); //удаляет выбранную категорию
    void addCategory(); //добавляет новую категорию, родительским элементом является выбранная категория
    void categorySettings(); //вызывает окно свойств категорий
    void updateTreeModel(const QString cat_name, int row, int parent_id, int cat_id); //добавляет/обновляет строку категории в дереве категорий
    void setTaskCnt(); //устанавливает кол-во одновременных закачек
    void showTaskDialog(); //показывает диалог состояния выделенного задания
    void showTaskDialog(int id_row); //переопределение метода
    void closeTaskDialog(); //удаляет закрытый диалог состояния задания

signals:
    void transAct();
    void needExecQuery(const QString &query);

private:
    void lockProcess(bool flag=true); //позволяет создать/удалить файл блокировки процесса
    void createInterface(); //настраивает элемкнты графического интерфейса
    void setEnabledTaskMenu(bool stat=false); //активирует/деактивирует меню для выделенных задач

    Ui::REXWindow *ui;
    QSystemTrayIcon *trayicon; //объект системного лотка
    QMovie *movie; //мувик для реализауии анимации в трее
    SettingsDialog *settDlg;

    PluginManager *plugmgr;
    QStringList pluginDirs; //список с директориями, в которых могут быть плагины
    QHash<int,QStringList> mesqueue; //очередь сообщений
    QHash<int,int> tasklist; //список дескрипторов активных заданий (id_in_table, id_task)
    QHash<int,TaskDialog*> dlglist;
    QString apphomedir; //путь к рабочему каталогу приложения, где хранятся все его файлы конфигураций
    QString dbconnect; //имя подклюения к БД
    bool sched_flag; //признак разрешения работы планировщика

    QHash<int,QString> plugfiles; //хэш путей к файлам плагинов
    QHash<int,LoaderInterface*> pluglist; //хэш ссылок на плагины
    QHash<QString,int> plugproto; //хэш дескрипторов плагинов с соответствующими протоколами
    TItemModel *model; //модель для работы с данными БД задач
    TreeItemModel *treemodel; //модель информационного дерева
    QSortFilterProxyModel *sfmodel; //прокси-модель для сортировки

    QToolButton *spdbtn; //кнопка выбора скорости загрузки
    QToolButton *taskbtn;
    QString downDir; //директория для загрузки файлов по умолчанию

    QString clip_last; //последняя строка в бюфере обьмена, на которую была реакция

    int max_tasks; //максимальное количество одновременных закачек
    int max_threads; //максимальное кол-во потоков при скачивании
    qint64 down_speed; //максимальная общая скорость скачивания (в кбит/с)
    Qt::WindowStates preStat; //предназначено для хранения предыдущего состояния окна (необходимо для реализации метода загрузки состояния окна)
    bool stop_flag; //флаг остановки всех заданий при завершении работы программы

    QByteArray plug_state;
};

#endif // REXWINDOW_H
