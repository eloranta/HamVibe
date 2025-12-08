#include <QtTest>

#include "../../spotparser.h"

class SpotParserTest : public QObject
{
    Q_OBJECT

private slots:
    void parsesTestLine1();
    void parsesTestLine2();
    void parsesTestLine3();
 };

namespace {
QString buildLine(const QString &spotter,
                  const QString &freq,
                  const QString &call,
                  const QString &message,
                  const QString &time)
{
    // Simple builder matching the regex-based parser expectations.
    return QString("DX de %1: %2 %3 %4 %5").arg(spotter, freq, call, message, time);
}
} // namespace

void SpotParserTest::parsesTestLine1()
{
    const QString line = "DX de OK1R:     432174.0  OK1JBR       FT8 CQ CQ CQ                   1430Z";
    const std::optional<ParsedSpot> parsed = ParseInputString(line);
    QVERIFY(parsed.has_value());
    QCOMPARE(parsed->spotter, QString("OK1R"));
    QCOMPARE(parsed->freq, QString("432174.0"));
    QCOMPARE(parsed->call, QString("OK1JBR"));
    QCOMPARE(parsed->message, QString("FT8 CQ CQ CQ"));
    QCOMPARE(parsed->time, QString("1430"));
}

void SpotParserTest::parsesTestLine2()
{
    const QString line = "DX de HL2/KC6STQ:  28387.0  KB3BAA     QRZ.COM: KB3BAA IS SILENT KEY  1436Z";
    const std::optional<ParsedSpot> parsed = ParseInputString(line);
    QVERIFY(parsed.has_value());
    QCOMPARE(parsed->spotter, QString("HL2/KC6STQ"));
    QCOMPARE(parsed->freq, QString("28387.0"));
    QCOMPARE(parsed->call, QString("KB3BAA"));
    QCOMPARE(parsed->message, QString("QRZ.COM: KB3BAA IS SILENT KEY"));
    QCOMPARE(parsed->time, QString("1436"));
}

void SpotParserTest::parsesTestLine3()
{
    const QString line = "DX de S51ZO:  10368070.0  S55ZMS/B     JN86cr 599                     1547Z";
    const std::optional<ParsedSpot> parsed = ParseInputString(line);
    QVERIFY(parsed.has_value());
    QCOMPARE(parsed->spotter, QString("S51ZO"));
    QCOMPARE(parsed->freq, QString("10368070.0"));
    QCOMPARE(parsed->call, QString("S55ZMS/B"));
    QCOMPARE(parsed->message, QString("JN86cr 599"));
    QCOMPARE(parsed->time, QString("1547"));
}

QTEST_MAIN(SpotParserTest)
#include "tst_spotparser.moc"
