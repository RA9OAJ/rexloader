#include <QtGui/QApplication>
#include <QTextCodec>
#include "noticewindow.h"
#include "noticemanager.h"

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

    QApplication a(argc, argv);
    /*NoticeWindow w;
    //w.show();
    w.setDisplayTime(10000);
    w.showNotice("REXLoader","Файл SETUP.EXE успечно закачан.",NoticeWindow::WT_Info);*/
    NoticeManager w;
    w.showNotice("REXLoader","Файл SETUP.EXE успечно закачан.",NoticeWindow::WT_Info);
    w.showNotice("REXLoader","Файл SETUP.EXE успечно закачан.",NoticeWindow::WT_Info);
    w.showNotice("REXLoader","Файл SETUP.EXE успечно закачан.",NoticeWindow::WT_Info);
    w.showNotice("REXLoader","Файл SETUP.EXE успечно закачан.",NoticeWindow::WT_Info);
    w.showNotice("REXLoader","Файл SETUP.EXE успечно закачан.",NoticeWindow::WT_Info);

    return a.exec();
}
