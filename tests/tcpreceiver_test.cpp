#include <QtTest/QtTest>
#include <tuple>

std::tuple<const QString, const QString, const QString, const QString, const QString> parseLine(const QString &line);

class TcpReceiverParseLineTest : public QObject
{
    Q_OBJECT

private slots:
    // void parsesLineWithZuluSuffixTime();
    // void parsesLineWithPlainTime();
    void parseNormalLine();
    void parseLongerLine();
};

// static QString makeClusterLineWithZuluTime(const QString &sender,
//                                            const QString &freq,
//                                            const QString &call,
//                                            const QString &msg,
//                                            const QString &timeZ)
// {
//     QString line(80, QLatin1Char(' '));
//     line.replace(0, 6, "DX de ");
//     line.replace(6, sender.size(), sender);
//     line[6 + sender.size()] = QLatin1Char(':');
//     line.replace(13, qMin(freq.size(), 11), freq.left(11));
//     line.replace(26, qMin(call.size(), 13), call.left(13));
//     line.replace(39, qMin(msg.size(), 30), msg.left(30));
//     line.replace(69, qMin(timeZ.size(), 5), timeZ.left(5));
//     return line;
// }

// static QString makeClusterLineWithPlainTime(const QString &sender,
//                                             const QString &freq,
//                                             const QString &call,
//                                             const QString &msg,
//                                             const QString &time)
// {
//     QString line(80, QLatin1Char(' '));
//     line.replace(0, 6, "DX de ");
//     line.replace(6, sender.size(), sender);
//     line[6 + sender.size()] = QLatin1Char(':');
//     line.replace(13, qMin(freq.size(), 11), freq.left(11));
//     line.replace(26, qMin(call.size(), 13), call.left(13));
//     line.replace(39, qMin(msg.size(), 30), msg.left(30));
//     line.replace(70, qMin(time.size(), 4), time.left(4));
//     return line;
// }

// void TcpReceiverParseLineTest::parsesLineWithZuluSuffixTime()
// {
//     const QString line = makeClusterLineWithZuluTime("OH2ABC", "14074.0", "OG3Z", "CQ FT8", "1234Z");
//     const auto [sender, freq, call, msg, time] = parseLine(line);

//     QCOMPARE(sender, QString("OH2ABC"));
//     QCOMPARE(freq, QString("14074.0"));
//     QCOMPARE(call, QString("OG3Z"));
//     QCOMPARE(msg, QString("CQ FT8"));
//     QCOMPARE(time, QString("1234"));
// }

// void TcpReceiverParseLineTest::parsesLineWithPlainTime()
// {
//     const QString line = makeClusterLineWithPlainTime("N0CALL", "  7020.5", "K1ABC", "TEST CW", "2359");
//     const auto [sender, freq, call, msg, time] = parseLine(line);

//     QCOMPARE(sender, QString("N0CALL"));
//     QCOMPARE(freq, QString("7020.5"));
//     QCOMPARE(call, QString("K1ABC"));
//     QCOMPARE(msg, QString("TEST CW"));
//     QCOMPARE(time, QString("2359"));
// }

void TcpReceiverParseLineTest::parseNormalLine()
{
    const QString line = "DX de PC4Y:      14024.8  SP2QG        CW                             1324Z";
    const auto [sender, freq, call, msg, time] = parseLine(line);
    QCOMPARE(sender, QString("PC4Y"));
    QCOMPARE(freq, QString("14024.8"));
    QCOMPARE(call, QString("SP2QG"));
    QCOMPARE(msg, QString("CW"));
    QCOMPARE(time, QString("1324"));
}

void TcpReceiverParseLineTest::parseLongerLine()
{
    const QString line = "DX de OL25PRADED:  14277.0  OL25PRADED  cq cq cq                       1324Z";
    const auto [sender, freq, call, msg, time] = parseLine(line);
    QCOMPARE(sender, QString("OL25PRADED"));
    QCOMPARE(freq, QString("14277.0"));
    QCOMPARE(call, QString("OL25PRADED"));
    QCOMPARE(msg, QString("cq cq cq"));
    QCOMPARE(time, QString("1324"));
}


QObject *createTcpReceiverTest()
{
    return new TcpReceiverParseLineTest;
}

#include "tcpreceiver_test.moc"
