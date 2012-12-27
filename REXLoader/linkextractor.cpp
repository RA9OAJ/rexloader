#include "linkextractor.h"
#include <QtCore>
#include <QWebElementCollection>
#include <QWebElement>
#include <QWebFrame>

LinkExtractor::LinkExtractor(QObject *parent) :
    QObject(parent)
{
    mp_web_view = new QWebView();
}


LinkExtractor::~LinkExtractor()
{
    delete mp_web_view;
}


void LinkExtractor::setText(const QString &text)
{
    //m_text = text;
    mp_web_view->setHtml(text);
    m_link_list.clear();
}


QList<ResourceLink> LinkExtractor::extract()
{
    // шаблон для html ссылок
    // setMinimal(true) надо для разделения подряд идущих ссылок
    // если false то 2 и более подряд идущие ссылки воспримутся как одна
    //QRegExp reg_ex_a(QString::fromUtf8("<a.+href=[\"']([a-zA-Z:/%0-9&]*)[\"'].*>(.*)</a>"));
//    QRegExp reg_ex_a(QString::fromUtf8("<a.+href=[\"'](.*)[\"'].*>(.*)</a>"));
//    reg_ex_a.setMinimal(true);

    // шаблон для ссылок записаных как текст
//    QRegExp reg_ex_arbitrary_link(QString::fromUtf8("((?:http|https)://[^\\s\\n\"'>]+)"));


    // поиск html ссылок
//    int pos = 0;
//    while ((pos = reg_ex_a.indexIn(m_text, pos)) != -1) {
//        addALink(reg_ex_a.cap(2), reg_ex_a.cap(1));
//        pos += reg_ex_a.matchedLength();
//    }


    // поиск текстовых ссылок
//    pos = 0;
//    while ((pos = reg_ex_arbitrary_link.indexIn(m_text, pos)) != -1) {
//        addArbitraryLink(reg_ex_arbitrary_link.cap(1));
//        pos += reg_ex_arbitrary_link.matchedLength();
//    }


    //mp_web_view->setHtml(m_text);
    QWebElementCollection elements = mp_web_view->page()->mainFrame()->findAllElements("a");
    foreach (QWebElement e, elements) {
        addALink(e.attribute(QString::fromUtf8("href")),e.attribute(QString::fromUtf8("href")));
    }


    return m_link_list;
}


void LinkExtractor::addALink(const QString &name, const QString &url)
{
    ResourceLink res_link;
    res_link.type = ResourceLink::A;
    res_link.name = name;
    res_link.url = url;
    m_link_list.append(res_link);
}


void LinkExtractor::addArbitraryLink(const QString &url)
{
    ResourceLink res_link;
    res_link.type = ResourceLink::ARBITRARY ;
    res_link.name = "";
    res_link.url = url;
    m_link_list.append(res_link);
}
