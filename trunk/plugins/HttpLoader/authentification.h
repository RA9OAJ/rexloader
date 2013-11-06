#ifndef AUTHENTIFICATION_H
#define AUTHENTIFICATION_H

#include <QString>
#include <QStringList>
#include <QCryptographicHash>
#include <QRegExp>
#include <QVariant>
#include <QUrl>
#include <QHash>

class Authentification
{
public:
    Authentification();
    QString getAuthString(const QUrl &url, const QString &body = QString());
    void setServerAuthData(const QString &server_req);
    QHash<QString, QVariant> getOptions() const;
    QVariant option(const QString &opt) const;
    void setUsername(const QString &user);
    void setPassword(const QString &passwd);

private:
    enum AuthMethod
    {
        Digest_MD5,
        Basic
    };

    QString md5Digest();
    void parseHttpHeader(const QString &hdr);
    QString unquote(const QString &val) const;

    QHash<QString,QVariant> options;
};

#endif // AUTHENTIFICATION_H
