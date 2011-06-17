#include <QtGui/QApplication>
#include "rexwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    REXWindow w;
    w.show();

    return a.exec();
}
