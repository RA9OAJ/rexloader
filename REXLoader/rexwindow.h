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
    void startTrayIconAnimaion();
    void stopTrayIconAnimation();
    void scanTaskQueue(); //сканирует очередь заданий и при обнаружении нового доступного, начинает скачивание
    void scanNewTaskQueue(); //сканирует очередь url для добавления в очередь заданий
    void scanClipboard(); //сканирует буфер обмена на наличие доступного для скачивания URL

protected:
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent *e);
    void openDataBase(); //создает/открывает существующую базу данных закачек
    void saveSettings(); //сохраняет свойства приложения
    void loadSettings(); //загружает свйоства приложения
    int loadPlugins(); //сканирует директории в поисках доступных плагинов, возвращает количество найденных и загруженных плагинов

protected slots:
    void scheuler(); //внутренний планировщик с интервалом в 1 секунду
    void updateTrayIcon();
    void showAddTaskDialog();
    void showHideSlot(QSystemTrayIcon::ActivationReason type);

private:
    void lockProcess(bool flag=true); //позволяет создать/удалить файл блокировки процесса
    void createInterface(); //настраивает элемкнты графического интерфейса

    Ui::REXWindow *ui;
    QSystemTrayIcon *trayicon; //объект системного лотка

    QStringList pluginDirs; //список с директориями, в которых могут быть плагины
    QHash<int,QStringList> *mesqueue; //очередь сообщений
    QList<int> *tasklist; //список дескрипторов активных заданий
    QString apphomedir; //путь к рабочему каталогу приложения, где хранятся все его файлы конфигураций
    QString dbconnect; //имя подклюения к БД
    bool sched_flag; //признак разрешения работы планировщика

    QHash<int,QString> plugfiles; //хэш путей к файлам плагинов
    QHash<int,LoaderInterface*> pluglist; //хэш ссылок на плагины
    QHash<QString,int> plugproto; //хэш дескрипторов плагинов с соответствующими протоколами
    TItemModel *model; //модель для работы с данными БД задач
    QSortFilterProxyModel *sfmodel; //прокси-модель для сортировки

    QToolButton *spdbtn; //кнопка выбора скорости загрузки
    QString downDir; //директория для загрузки файлов по умолчанию

    QString clip_last; //последняя строка в бюфере обьмена, на которую была реакция

    int max_tasks; //максимальное количество одновременных закачек
    bool clip_autoscan; //признак разрешения автоматического сканирования буфера обмена на доступные ссылки
};

#endif // REXWINDOW_H
