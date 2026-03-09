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
    QString data;

    // data =
    //     "Namibia:                  38:  57:  AF:  -22.00:   -17.00:    -1.0:  V5:\n"
    //     "V5,=V51AS/L,=V51NAM/L,=V51NAM/LH,=V51WW/L,=V51WW/LH,=V55V/LH,=V59PP/L,=V59SWK/L,=V59SWK/LH;";
    // country.ParseCty(data);
    // QCOMPARE(country.GetCountry("V55Y"), QString("Namibia"));

    // data =
    //     "Wales:                    14:  27:  EU:   52.28:     3.73:     0.0:  GW:\n"
    //     "    =2O0RMR,=2O0RWF,=2O0TRR,=2O0UAA,=2O0WDS,=2O0ZJA,=2O12W,=2Q0CDY,=2Q0CGM,=2Q0CLJ,=2Q0CVE,=2Q0DAA;";
    // country.ParseCty(data);
    // QCOMPARE(country.GetCountry("GW0A"), QString("Wales"));

    // data =
    //     "United States:            05:  08:  NA:   37.60:    91.87:     5.0:  K:\n"
    //     "    AA,AB,AC,AD,AE,AF,AG,AI,AJ,AK,K,N,W,=N2NL/MM(7),=NH7RO/M,=NM5RC/P,=NQ4I/AM,=YL3IZ/MM;";
    // country.ParseCty(data);
    // QCOMPARE(country.GetCountry("AC2FA"), QString("United States"));

    // data =
    //     "Slovak Republic:          15:  28:  EU:   49.00:   -20.00:    -1.0:  OM:\n"
    //     "OM;";
    // country.ParseCty(data);
    // QCOMPARE(country.GetCountry("OM5AY"), QString("Slovak Republic"));

    //qDebug() << country.prefixMap;
    data =
        "Bouvet:                   38:  67:  AF:  -54.42:    -3.38:    -1.0:  3Y/b:\n"
        "    =3Y0K,=3Y/ZS6GCM,=3Y0C,=3Y0E,=3Y0J,=3Y7GIA,=3Y7THA;";
    country.ParseCty(data);
    QCOMPARE(country.GetCountry("3Y0K"), QString("Bouvet"));
    qDebug() << country.callMap;
    qDebug() << country.prefixMap;
}

#include "country_test.moc"
