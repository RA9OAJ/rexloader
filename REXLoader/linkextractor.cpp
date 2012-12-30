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
    mp_web_view->setHtml(text);
    m_link_list.clear();
}


QList<ResourceLink> LinkExtractor::extract()
{
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
