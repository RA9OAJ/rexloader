#ifndef LINKEXTRACTOR_H
#define LINKEXTRACTOR_H

#include <QObject>
#include <QWebView>

class ResourceLink{
public:
    enum LinkType {A, ARBITRARY, IMG, CSS, JAVASCRIPT};
    LinkType type;
    QString name;
    QString url;
    QString protocol;
};


class LinkExtractor : public QObject
{
    Q_OBJECT
public:
    explicit LinkExtractor(QObject *parent = 0);
    ~LinkExtractor();
    void setText(const QString &text);
    QList<ResourceLink> extract();
private:
    QList<ResourceLink> m_link_list;
    QWebView *mp_web_view;
    // добавляем в список ссылок найденую ссылку A
    void addALink(const QString &name, const QString &url);
    void addArbitraryLink(const QString &url);
};

#endif // LINKEXTRACTOR_H
