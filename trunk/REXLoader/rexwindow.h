#ifndef REXWINDOW_H
#define REXWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QStringList>
#include <QHash>
#include <QList>

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
    //void showMessage(const QString &title, const QString &text, int rang = 50,QSystemTrayIcon::MessageIcon type = QSystemTrayIcon::Information);

public slots:
    //void updateTaskSheet(); //обновляет содержимое таблицы списка заданий

protected:
    void changeEvent(QEvent *e);
    void openDataBase(); //создает/открывает существующую базу данных закачек
    void saveSettings(); //сохраняет свойства приложения
    void loadSettings(); //загружает свйоства приложения
    //int loadPlugins(); //сканирует директории в поисках доступных плагинов, возвращает количество найденных и загруженных плагинов.

protected slots:
    //void scheuler();

private:
    Ui::REXWindow *ui;
    QSystemTrayIcon *trayicon;

    QStringList pluginDirs; //список с директориями, в которых могут быть плагины
    QHash<int,QStringList> *mesqueue; //очередь сообщений
    QList<int> *tasklist; //список дескрипторов активных заданий

};

#endif // REXWINDOW_H
