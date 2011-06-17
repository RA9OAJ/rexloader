#ifndef REXWINDOW_H
#define REXWINDOW_H

#include <QMainWindow>

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

protected:
    void changeEvent(QEvent *e);
    void openDataBase();
    void saveSettings();
    void loadSettings();

private:
    Ui::REXWindow *ui;
};

#endif // REXWINDOW_H
