#include <QtGui/QApplication>
#include <QTextCodec>
#include "dialog.h"

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QApplication a(argc, argv);

    QTranslator translator;
    translator.load("test_ru");
    a.installTranslator(&translator);

    Dialog w;
    w.show();

    return a.exec();
}
