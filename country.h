#ifndef COUNTRY_H
#define COUNTRY_H

#include <QString>

class Country
{
public:
    Country() = default;

    void init();
    void ParseCty(const QString &content);
    QString GetCountry(const QString &call) const;
};

#endif // COUNTRY_H
