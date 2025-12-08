#include <QtTest>

#include "../../spotparser.h"

class SpotParserTest : public QObject
{
    Q_OBJECT

private slots:
    void parsesTestLine();
    // void parsesNominalLine();
    // void rejectsMissingColon();
};

namespace {
QString buildLine(const QString &spotter,
                  const QString &freq,
                  const QString &call,
                  const QString &message,
                  const QString &time)
{
    QString line(80, ' ');
    line.replace(0, 6, "DX de ");
    line.replace(6, spotter.size(), spotter);
    line[6 + spotter.size()] = ':';
    line.replace(13, freq.size(), freq);
    line.replace(28, call.size(), call);
    line.replace(38, message.size(), message);
    line.replace(70, time.size(), time);
    return line.trimmed();
}
} // namespace

// void SpotParserTest::parsesNominalLine()
// {
//     const QString line = buildLine("EA1FGX", "14074.0", "EA1FGX", "FT8 CQ TEST", "1234");
//     const std::optional<ParsedSpot> parsed = ParseInputString(line);
//     QVERIFY(parsed.has_value());
//     QCOMPARE(parsed->spotter, QString("EA1FGX"));
//     QCOMPARE(parsed->freq, QString("432174.0"));
//     QCOMPARE(parsed->call, QString("EA1FGX"));
//     QCOMPARE(parsed->message, QString("FT8 CQ TEST"));
//     QCOMPARE(parsed->time, QString("1234"));
// }

void SpotParserTest::parsesTestLine()
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

// void SpotParserTest::rejectsMissingColon()
// {
//     QString line = buildLine("EA1FGX", "14074.0", "EA1FGX", "FT8 CQ TEST", "1234");
//     line.replace(6 + QString("EA1FGX").size(), 1, ' '); // drop colon
//     const std::optional<ParsedSpot> parsed = ParseInputString(line);
//     QVERIFY(!parsed.has_value());
// }

QTEST_MAIN(SpotParserTest)
#include "tst_spotparser.moc"
