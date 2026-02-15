#include <QtTest/QtTest>

#include "country.h"

class CountryTest : public QObject
{
    Q_OBJECT
private slots:
    void parseAndLookup();
};

QObject *createCountryTest()
{
    return new CountryTest();
}

void CountryTest::parseAndLookup()
{
    Country country;
    const QString data =
        "Yemen:                    21:  39:  AS:   15.65:   -48.12:    -3.0:  7O:\n"
        "    7O,\n"
        "    =7O/DL7ZM(37)[48],=7O2A(37)[48],=7O6T(37)[48],=7O73T(37)[48],=7O8AD(37)[48],=7O8AE(37)[48];\n";
    country.ParseCty(data);

    QCOMPARE(country.GetCountry("7O2A"), QString("Yemen"));
    QCOMPARE(country.GetCountry("7O/DL7ZM"), QString("Yemen"));
    QCOMPARE(country.GetCountry("7O8AD"), QString("Yemen"));
    QCOMPARE(country.GetCountry("7OAAA"), QString("Yemen"));
}

#include "country_test.moc"
