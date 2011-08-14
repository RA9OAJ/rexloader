#ifndef NOTICEWINDOW_H
#define NOTICEWINDOW_H

#include <QtGui/QDialog>
#include <QToolBar>
#include <QPushButton>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include <QDesktopWidget>
#include <QDebug>
#include <QBitmap>
#include <QLayout>
#include <QTextBrowser>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QScrollArea>
#include <QFrame>

#ifdef Q_WS_X11
#  include <X11/Xlib.h>
#  include <X11/Xatom.h>
#  include <QX11Info>
#endif

class NoticeWindow : public QDialog
{
    Q_OBJECT

public:
    enum WindowType{
        WT_Info,        /*0*/
        WT_Warning,
        WT_Critical,
        WT_Manual       /*3*/
    };

    enum ShowEffectType{
        SE_WithoutEffect, /*0*/
        SE_PopUp,         /*1*/
        SE_Approximation, /*2*/
        SE_Appearance     /*3*/
    };

    NoticeWindow(QWidget *parent = 0);
    virtual ~NoticeWindow();

public slots:
    void showNotice(const QString &title, const QString &message, WindowType wtype = WT_Info);
    void setDisplayTime(const int sec);
    void setShowEffect(ShowEffectType etype = SE_PopUp);
    void setCloseEffect(ShowEffectType etype = SE_PopUp);
    void setSpeedOfEffects(int speed = 5); //Скорость от 0...10
    void closeNotice();
    void setOffsetPos(int x, int y);
    void setDOffsetPos(int x, int y);

private slots:
    void showEffect();
    void approximationShowEffect();
    void appearanceShowEffect();
    void closeEffect();
    void approximationCloseEffect();
    void appearanceCloseEffect();
    void switchDiaplay();

private:
    void createWidgets();
    void createSettingsWidgets();
    void setWindowStyle();
    void moveToAllDesktops(bool _flag = true);

    QToolBar *toolbar;
    QPushButton *btn1;
    QPushButton *btn2;
    QLabel *ltitle;
    QTextBrowser *textbrowser;
    QPushButton *pExec;
    QPushButton *pOpenDir;

    QGroupBox *grp1;
    QGroupBox *grp2;
    QVBoxLayout *lay1;
    QVBoxLayout *lay2;
    QVBoxLayout *lay3;
    QGridLayout *lay4;
    QHBoxLayout *lay5;
    QFrame *frame;
    QScrollArea *sarea1;
    QLabel *lbl1;
    QLabel *lbl2;

    QCheckBox *onalldesk;
    QRadioButton *popupEff;
    QRadioButton *appearEffect;
    QRadioButton *approxEffect;
    QRadioButton *noEffect;
    QRadioButton *lefttop;
    QRadioButton *righttop;
    QRadioButton *leftbottom;
    QRadioButton *rightbottom;
    QSpinBox *deltaX;
    QSpinBox *deltaY;
    QSpinBox *winWidth;
    QSpinBox *winHeight;

    QPushButton *btnOK;
    QPushButton *btnCancel;

    QRect base;
    int disp_time;
    int show_effect;
    int close_effect;
    int effects_speed;
    int dx;
    int dy;
    int ddx;
    int ddy;
    int diff;
    double diff_opac;
    bool desktopflag;
    bool pleft, ptop;
    QTime ts;
    QTime tw;
    QTime twf;

};

#endif // NOTICEWINDOW_H
