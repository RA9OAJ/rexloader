#include <QtGui/QApplication>
#include <QTextCodec>
#include "noticewindow.h"

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);
    NoticeWindow w;
    //w.show();
    w.showNotice("REXLoader","");

    return a.exec();
}
