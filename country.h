#ifndef COUNTRY_H
#define COUNTRY_H

#include <QString>
#include <QMap>

class Country
{
public:
    Country() = default;

    void init();
    void ParseCty(const QString &content);
    QString GetCountry(const QString &call) const;
private:
    QMap<QString, QString> callMap;
    QMap<QString, QString> prefixMap;
};

#endif // COUNTRY_H
