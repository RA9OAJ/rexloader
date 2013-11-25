#include "authentification.h"
#include <QDebug>

Authentification::Authentification()
{
    options.clear();
}

QString Authentification::getAuthString(const QUrl &url, const QString &body)
{
    if(options.isEmpty())
        return QString();

    if(option("_method").toInt() == Digest_MD5)
    {
        options["uri"] = "\"" + url.encodedPath() + "\"";
        options["_entity_body"] = body;
        return md5Digest();
    }
    else if(option("_method").toInt() == Basic)
        return basic();

    return QString();
}

void Authentification::setServerAuthData(const QString &server_req)
{
    options.clear();
    parseHttpHeader(server_req);
}

QHash<QString, QVariant> Authentification::getOptions() const
{
    return options;
}

QVariant Authentification::option(const QString &opt) const
{
    return options.value(opt);
}

void Authentification::setUsername(const QString &user)
{
    options["_username"] = "\"" + user + "\"";
}

void Authentification::setPassword(const QString &passwd)
{
    options["_password"] = passwd;
}

bool Authentification::isEmpty() const
{
    return options.isEmpty();
}

QString Authentification::md5Digest()
{
    options["_nc"] = options["_nc"].toInt() + 1;
    QString h1,h2;
    QCryptographicHash hash(QCryptographicHash::Md5);
    /*qDebug()<<"h1="<<QString("%1:%2:%3").arg(unquote(option("_username").toString()),
                                             unquote(option("realm").toString()),
                                             option("_password").toString());*/
    hash.addData((QString("%1:%2:%3").arg(unquote(option("_username").toString()),
                                          unquote(option("realm").toString()),
                                          option("_password").toString()).toAscii()));

    h1 = hash.result().toHex();
    hash.reset();

    QString qop = option("qop").toString();
    if(qop.indexOf(QRegExp("auth[^-]{1}")) != -1 || qop.isEmpty())
    {
        /*qDebug()<<"h2="<<(QString("%1:%2").arg("GET",unquote(option("uri").toString())));*/
        hash.addData((QString("%1:%2").arg("GET",unquote(option("uri").toString()))).toAscii());
        h2 = hash.result().toHex();
    }
    else if(qop.indexOf("auth-int") != -1)
    {
        hash.addData(option("_entity_body").toString().toAscii());
        QString entity_body = hash.result().toHex();

        hash.reset();
        hash.addData((QString("%1:%2:%3").arg("GET",unquote(option("uri").toString()),entity_body)).toAscii());
        h2 = hash.result();
    }
    else return QString();

    hash.reset();
    QString tnc = QString::number(options["_nc"].toInt());
    options["nc"] = QString("0000000%1").arg(tnc).mid(tnc.size() - 1);
    int icnonce = qrand();
    options["cnonce"] = "\"" + QByteArray((char*)&icnonce,sizeof(icnonce)).toHex() + "\"";
    hash.addData((QString("%1:%2:%3:%4:%5:%6").arg(h1,
                 unquote(option("nonce").toString()),
                 options["nc"].toString(),
                 unquote(options["cnonce"].toString()),
                 unquote(options["qop"].toString()),
                 h2
                 ).toAscii()));
    /*qDebug()<<"result="<<(QString("%1:%2:%3:%4:%5:%6").arg(h1,
                                                           unquote(option("nonce").toString()),
                                                           options["nc"].toString(),
                                                           unquote(options["cnonce"].toString()),
                                                           unquote(options["qop"].toString()),
                                                           h2
                                                           ));*/
    options["response"] = QString("\"%1\"").arg(QString(hash.result().toHex()));

    QString out = QString(" Digest username=%1,").arg(options.value("_username").toString());
    QStringList keys = options.keys();
    foreach(QString key, keys)
    {
        if(key.mid(0,1) == "_")
            continue;

        out += QString(" %1=%2,").arg(key,options.value(key).toString());
    }
    out = out.mid(0,out.size()-2);

    return out;

}

QString Authentification::basic()
{
    if(options.value("_username").toString().isEmpty() || options.value("_password").toString().isEmpty())
        return QString();

    QString base64_auth = QString("%1:%2").arg(options.value("_username").toString(),
                                                  options.value("_password").toString()).toAscii().toBase64();
    return QString(" Basic %1").arg(base64_auth);
}

void Authentification::parseHttpHeader(const QString &hdr)
{
    int bpos = hdr.indexOf(QRegExp("Basic",Qt::CaseInsensitive));
    int dpos = hdr.indexOf(QRegExp("Digest",Qt::CaseInsensitive));
    if(dpos == -1 && bpos == -1)
        return;

    options["_method"] = (bpos != -1 ? (int)Basic : (int)Digest_MD5);
    int spos = hdr.indexOf(" ",(bpos != -1 ? bpos : dpos));
    if(spos == -1)
    {
        options.clear();
        return;
    }

    QString inhdr = hdr.mid(spos);
    QStringList params = inhdr.split(",");

    foreach (QString cur, params)
    {
        cur = cur.replace(QRegExp("^\\s+"),"");
        cur = cur.replace(QRegExp("\\s+$"),"");

        spos = cur.indexOf("=");
        if(spos == -1)
            continue;

        options[cur.mid(0,spos)] = cur.mid(spos + 1);
    }
}

QString Authentification::unquote(const QString &val) const
{
    QString out = val;
    out = out.replace(QRegExp("^\"{1}"),"");
    out = out.replace(QRegExp("\"{1}$"),"");

    return out;

}
