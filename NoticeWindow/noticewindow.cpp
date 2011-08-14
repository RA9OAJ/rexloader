#include "noticewindow.h"

NoticeWindow::NoticeWindow(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_QuitOnClose);
    setWindowTitle("NoticeWindow");
    setMaximumHeight(120);
    setMaximumWidth(250);
    resize(maximumWidth(),1);

    dx = dy = 3;
    disp_time = 3000;
    show_effect = SE_PopUp;
    close_effect = SE_PopUp;
    effects_speed = 7; /* 10 - это полное появление за 3 секунды, 1 - за 0,3 секунды*/
    diff = (float)maximumSize().height()/(2000/effects_speed/15);
    if(!diff)diff = 1;
    diff_opac = 1.0/(double)(2000/effects_speed/15);
    if(diff_opac < 0.005)diff_opac = 0.005;
    desktopflag = true;
    ptop = false;
    pleft = false;

    QDesktopWidget ds;
    base = ds.availableGeometry();
    move(base.topLeft().x()+base.width()-size().width()-dx, base.topLeft().y()+base.height()-dy);

    createWidgets();
    createSettingsWidgets();
    setWindowStyle();

    setWindowOpacity(0.01);
    //moveToAllDesktops();
}

void NoticeWindow::moveToAllDesktops(bool _flag)
{
#ifdef Q_WS_X11
    Atom atom = XInternAtom(QX11Info::display(), "_NET_WM_DESKTOP", false);
    if (atom)
    {
        unsigned int val;
        if(_flag) val = 0xffffffff;
        else val = 0x2;
        XChangeProperty(QX11Info::display(), this->winId(), atom, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&val, 1);
    }
#endif
}

void NoticeWindow::setWindowStyle()
{
    setStyleSheet(".NoticeWindow {border: 1px solid black;"
                  "background: #ffffff;}");
}

void NoticeWindow::createWidgets()
{
    toolbar = new QToolBar(this);
    btn1 = new QPushButton(this);
    btn2 = new QPushButton(this);
    ltitle = new QLabel(this);
    textbrowser = new QTextBrowser(this);
    pExec = new QPushButton(this);
    pExec->setMaximumWidth(30);
    pExec->setMaximumHeight(20);
    pExec->setStyleSheet("QPushButton {border: none; image: url(:/noticewindow/run_27x20.png);}"
                            "QPushButton::hover {image: url(:/noticewindow/run1_27x20.png);}"
                            "QPushButton::pressed {image: url(:/noticewindow/run2_27x20.png);}"
                            "QPushButton::disabled {image: url(:/noticewindow/run3_27x20.png);}");
    pOpenDir = new QPushButton(this);
    pOpenDir->setMaximumWidth(30);
    pOpenDir->setMaximumHeight(20);
    pOpenDir->setStyleSheet("QPushButton {border: none; image: url(:/noticewindow/folder_27x20.png);}"
                            "QPushButton::hover {image: url(:/noticewindow/folder1_27x20.png);}"
                            "QPushButton::pressed {image: url(:/noticewindow/folder2_27x20.png);}"
                            "QPushButton::disabled {image: url(:/noticewindow/folder3_27x20.png);}");

    connect(btn1,SIGNAL(released()),this,SLOT(closeNotice()));
    connect(btn2,SIGNAL(released()),this,SLOT(switchDiaplay()));

    toolbar->setMinimumSize(maximumWidth(),23);
    toolbar->setMaximumWidth(maximumWidth());
    toolbar->setMaximumHeight(23);
    toolbar->setLayoutDirection(Qt::RightToLeft);
    toolbar->addWidget(btn1);
    toolbar->addWidget(btn2);
    toolbar->addWidget(ltitle);

    btn1->setStyleSheet("QPushButton {border: none; image: url(:/noticewindow/close1.png);}"
                        "QPushButton::hover {image: url(:/noticewindow/close2.png);}"
                        "QPushButton::pressed {image: url(:/noticewindow/close3.png);}");
    btn1->setMask(QBitmap(":/noticewindow/btnpixmap.png"));
    btn1->setMaximumSize(QBitmap(":/noticewindow/btnpixmap.png").size());

    btn2->setStyleSheet("QPushButton {border: none; image: url(:/noticewindow/settings1.png);}"
                        "QPushButton::hover {image: url(:/noticewindow/settings2.png);}"
                        "QPushButton::pressed {image: url(:/noticewindow/settings3.png);}");
    btn2->setMask(QBitmap(":/noticewindow/btnpixmap.png"));
    btn2->setMaximumSize(QBitmap(":/noticewindow/btnpixmap.png").size());

    ltitle->setText(windowTitle());
    ltitle->setStyleSheet("font: 12 bold; color: #ffffff;");
    ltitle->setAlignment(Qt::AlignCenter);
    ltitle->setMinimumWidth(toolbar->minimumWidth()-btn1->maximumWidth()-btn2->maximumWidth()-6);
    toolbar->setStyleSheet("QToolBar {background: #55cc55;"
                           "border-left: 1px solid black;"
                           "border-right: 1px solid black;"
                           "border-top: 1px solid black;}");

    textbrowser->move(textbrowser->pos().x()+1,toolbar->height());
    textbrowser->setMaximumWidth(size().width()-2);
    textbrowser->setMaximumHeight(maximumHeight()-toolbar->size().height()-2);
    textbrowser->resize(textbrowser->maximumSize());
    textbrowser->setStyleSheet("QTextBrowser {border: none; font: 12px; background: url(:/noticewindow/info.png) center no-repeat;}");
    pOpenDir->move(width()-pOpenDir->width()-2,textbrowser->height());
    pExec->move(width()-pOpenDir->width()-pExec->width()-2,textbrowser->height());
}

void NoticeWindow::createSettingsWidgets()
{
    frame = new QFrame(this);
    sarea1 = new QScrollArea(this);
    grp1 = new QGroupBox(tr("Effects"),frame);
    grp2 = new QGroupBox(tr("Position and Size"),frame);
    lay1 = new QVBoxLayout;
    lay2 = new QVBoxLayout;
    lay3 = new QVBoxLayout;
    lay4 = new QGridLayout;
    lay5 = new QHBoxLayout;

    btnOK = new QPushButton(tr("Ok"),this);
    onalldesk = new QCheckBox(tr("On all desktops"),this);
    onalldesk->setChecked(desktopflag);
    popupEff = new QRadioButton(this);
    popupEff->setText(tr("Pop-Up"));
    popupEff->setChecked(true);
    appearEffect = new QRadioButton(this);
    appearEffect->setText(tr("Appearance"));
    approxEffect = new QRadioButton(this);
    approxEffect->setText(tr("Approximation"));
    noEffect = new QRadioButton(this);
    noEffect->setText(tr("Without Effects"));
    lay1->addWidget(popupEff);
    lay1->addWidget(appearEffect);
    lay1->addWidget(approxEffect);
    lay1->addWidget(noEffect);
    grp1->setLayout(lay1);
    grp1->setStyleSheet("QGroupBox {font: 10px;} QRadioButton {font: 10px; height: 15px fixed;}");
    lay2->addWidget(grp1);
    lay2->addWidget(onalldesk);
    lay2->addWidget(btnOK);


    lefttop = new QRadioButton(tr("Lef top"),this);
    righttop = new QRadioButton(tr("Right top"),this);
    leftbottom = new QRadioButton(tr("Left bottom"),this);
    rightbottom = new QRadioButton(tr("Right bottom"),this);
    rightbottom->setChecked(true);
    lbl1 = new QLabel(tr("Offset X, Y"),this);
    lbl2 = new QLabel(tr("Size width, height"),this);
    btnCancel = new QPushButton(tr("Cancel"),this);
    connect(btnCancel,SIGNAL(released()),this,SLOT(switchDiaplay()));
    deltaX = new QSpinBox(this);
    deltaX->setValue(dx);
    deltaY = new QSpinBox(this);
    deltaY->setValue(dy);
    winWidth = new QSpinBox(this);
    winWidth->setMaximum(99999);
    winWidth->setValue(maximumWidth());
    winHeight = new QSpinBox(this);
    winHeight->setMaximum(99999);
    winHeight->setValue(maximumHeight());
    lay4->addWidget(lefttop,0,0);
    lay4->addWidget(righttop,0,1);
    lay4->addWidget(leftbottom,1,0);
    lay4->addWidget(rightbottom,1,1);
    lay4->addWidget(lbl1,2,0);
    lay4->addWidget(deltaX,3,0);
    lay4->addWidget(deltaY,3,1);
    lay4->addWidget(lbl2,4,0);
    lay4->addWidget(winWidth,5,0);
    lay4->addWidget(winHeight,5,1);
    grp2->setLayout(lay4);
    grp2->setStyleSheet("font: 10px;");
    lay3->addWidget(grp2);
    lay3->addWidget(btnCancel);

    lay5->addLayout(lay2);
    lay5->addLayout(lay3);
    frame->setLayout(lay5);
    frame->setStyleSheet("font: 10px;");


    sarea1->setWidget(frame);
    sarea1->move(4,sarea1->pos().y()+toolbar->height()+2);
    sarea1->setMaximumWidth(maximumWidth()-7);
    sarea1->setMaximumHeight(maximumHeight()-toolbar->size().height()-5);
    sarea1->setStyleSheet("QScrollArea {border: none; background: #ffffff;}");

    sarea1->setVisible(false);
}

void NoticeWindow::switchDiaplay()
{
    if(textbrowser->isVisible())
    {
        tw = QTime::currentTime();
        lefttop->setChecked(pleft && ptop);
        leftbottom->setChecked(pleft && !ptop);
        righttop->setChecked(!pleft && ptop);
        rightbottom->setChecked(!pleft && !ptop);

        switch(show_effect)
        {
        case SE_PopUp: popupEff->setChecked(true); break;
        case SE_Appearance: appearEffect->setChecked(true);break;
        case SE_Approximation: approxEffect->setChecked(true);break;
        default: noEffect->setChecked(true);break;
        }

    }
    else
    {
        twf = QTime::currentTime();
        QTimer::singleShot(disp_time-ts.secsTo(tw)*1000,this,SLOT(closeNotice()));
    }
    textbrowser->setVisible(!textbrowser->isVisible());
    sarea1->setVisible(!sarea1->isVisible());
}

void NoticeWindow::closeNotice()
{
    QPushButton *_btn = qobject_cast<QPushButton*>(sender());
    if(!textbrowser->isVisible() && _btn != btn1)return;
    if(!tw.isNull() && ts.secsTo(tw)+twf.secsTo(QTime::currentTime()) < disp_time/1000 && _btn != btn1 )return;


    switch(close_effect)
    {
    case SE_PopUp: closeEffect(); break;
    case SE_Approximation: approximationCloseEffect();break;
    case SE_Appearance: appearanceCloseEffect();break;
    case SE_WithoutEffect:

    default:
        close();break;
    }
}

void NoticeWindow::showNotice(const QString &title, const QString &message, WindowType wtype)
{
    if(desktopflag)moveToAllDesktops(true);
    setWindowTitle(title);
    ltitle->setText(title);
    textbrowser->setText(message);
    show();

    switch(show_effect)
    {
    case SE_PopUp: showEffect();break;
    case SE_Appearance: appearanceShowEffect();break;
    case SE_Approximation: approximationShowEffect();break;
    case SE_WithoutEffect:

    default:
        setWindowOpacity(1.0);
        move(pos().x(),pos().y()+maximumHeight());
        resize(maximumSize());
    }

    switch(wtype)
    {
    case WT_Info:
        textbrowser->setStyleSheet("QTextBrowser {border: none; font: 12px; background: url(:/noticewindow/info.png) center no-repeat;}");
        toolbar->setStyleSheet("QToolBar {background: #068c22;"
                               "border-left: 1px solid black;"
                               "border-right: 1px solid black;"
                               "border-top: 1px solid black;}");
        break;

    case WT_Warning:
        textbrowser->setStyleSheet("QTextBrowser {border: none; font: 12px; background: url(:/noticewindow/warning.png) center no-repeat;}");
        toolbar->setStyleSheet("QToolBar {background: #d2a800;"
                               "border-left: 1px solid black;"
                               "border-right: 1px solid black;"
                               "border-top: 1px solid black;}");
        break;

    case WT_Critical:
        textbrowser->setStyleSheet("QTextBrowser {border: none; font: 12px; background: url(:/noticewindow/error.png) center no-repeat;}");
        toolbar->setStyleSheet("QToolBar {background: #c20000;"
                               "border-left: 1px solid black;"
                               "border-right: 1px solid black;"
                               "border-top: 1px solid black;}");
        break;

    case WT_Manual:

    default:
        textbrowser->setStyleSheet("QTextBrowser {border: none; font: 12px;}");
        toolbar->setStyleSheet("QToolBar {background: #068c22;"
                               "border-left: 1px solid black;"
                               "border-right: 1px solid black;"
                               "border-top: 1px solid black;}");
        break;
    }

    showEffect();
}

void NoticeWindow::setDisplayTime(const int sec)
{
    disp_time = sec*1000;
}

void NoticeWindow::setShowEffect(ShowEffectType etype)
{
    show_effect = etype;
}

void NoticeWindow::setCloseEffect(ShowEffectType etype)
{
    close_effect = etype;
}

void NoticeWindow::setSpeedOfEffects(int speed)
{
    effects_speed = speed;
    diff = (float)maximumSize().height()/(2000/effects_speed/15);
    if(!diff)diff = 1;
    diff_opac = 1.0/(double)(2000/effects_speed/15);
    if(diff_opac < 0.005)diff_opac = 0.005;
}

void NoticeWindow::showEffect()
{
    if(size().height() >= maximumHeight()){QTimer::singleShot(disp_time,this,SLOT(closeNotice()));ts = QTime::currentTime();return;}
    if(windowOpacity() != 1.0)setWindowOpacity(1.0);
    resize(size().width(), size().height()+diff);
    move(pos().x(), base.topLeft().y()+base.height()-size().height()-dy);
    QTimer::singleShot(15,this,SLOT(showEffect()));
}

void NoticeWindow::closeEffect()
{
    if(size().height() <= 1){close();return;}
    if(!ptop)
    {
        resize(size().width(), size().height()-diff);
        move(pos().x(), base.topLeft().y()+base.height()-size().height()-dy);
    }
    else resize(size().width(), size().height()-diff);

    QTimer::singleShot(15,this,SLOT(closeEffect()));
}

void NoticeWindow::appearanceShowEffect()
{
    if(windowOpacity() >= 1.0){QTimer::singleShot(disp_time,this,SLOT(closeNotice()));return;}
    if(size().height()<=1)
    {
        move(base.topLeft().x()+base.width()-size().width()-dx, base.topLeft().y()+base.height()-maximumHeight()-dy);
        resize(size().width(),maximumHeight());
    }

    setWindowOpacity(windowOpacity()+diff_opac);
    QTimer::singleShot(15, this, SLOT(appearanceShowEffect()));
}

void NoticeWindow::appearanceCloseEffect()
{
    if(windowOpacity() <= 0.05){close();return;}
    setWindowOpacity(windowOpacity()-diff_opac);
    QTimer::singleShot(15, this, SLOT(appearanceCloseEffect()));
}

void NoticeWindow::approximationShowEffect()
{
    return;
}

void NoticeWindow::approximationCloseEffect()
{

}

NoticeWindow::~NoticeWindow()
{

}
