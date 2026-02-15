#ifndef COUNTRY_H
#define COUNTRY_H

#include <QString>
#include <QHash>

class Country
{
public:
    Country() = default;

    void init();
    void ParseCty(const QString &content);
    QString GetCountry(const QString &call) const;
public:
    QHash<QString, QString> callMap;
    QHash<QString, QString> prefixMap;
};

#endif // COUNTRY_H
