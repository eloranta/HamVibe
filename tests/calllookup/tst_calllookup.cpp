#include <QtTest>

#include "../../calllookup.h"

class CallLookupTest : public QObject
{
    Q_OBJECT

private slots:
    void picksLongestMatchingPrefix();
    void choosesTokenWithDigitsFirst();
    void returnsEmptyWhenNoMatch();
};

static QHash<QString, QString> buildMap()
{
    QHash<QString, QString> map;
    map.insert("EA", "Spain");
    map.insert("EA1", "Spain");
    map.insert("KC6", "Guam");
    map.insert("HL", "Korea");
    return map;
}

void CallLookupTest::picksLongestMatchingPrefix()
{
    const QHash<QString, QString> map = buildMap();
    QCOMPARE(findCountryForCall("EA1FGX", map), QString("SPAIN")); // prefers EA1 over EA
}

void CallLookupTest::choosesTokenWithDigitsFirst()
{
    const QHash<QString, QString> map = buildMap();
    // With HL2/KC6STQ the digit-bearing longest token KC6STQ should match KC6 -> Guam.
    QCOMPARE(findCountryForCall("HL2/KC6STQ", map), QString("GUAM"));
}

void CallLookupTest::returnsEmptyWhenNoMatch()
{
    const QHash<QString, QString> map = buildMap();
    QVERIFY(findCountryForCall("ZZ9PLURALZ", map).isEmpty());
}

QTEST_MAIN(CallLookupTest)
#include "tst_calllookup.moc"
