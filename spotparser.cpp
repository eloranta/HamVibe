#include "spotparser.h"

#include <QDebug>
#include <QRegularExpression>

std::optional<ParsedSpot> ParseInputString(const QString &line)
{
    static QRegularExpression re(R"(DX de\s+(\S+):\s+(\S+)\s+(\S+)\s+(.{0,39}?)\s+(\d{4})Z?)");
    QRegularExpressionMatch match = re.match(line);
    if (!match.hasMatch())
    {
        qDebug() << "Parse failed (regex) for line:" << line;
        return std::nullopt;
    }

    const QString spotter = match.captured(1);
    const QString freq = match.captured(2);
    const QString call = match.captured(3);
    const QString message = match.captured(4).trimmed();
    const QString time = match.captured(5);

    return ParsedSpot{spotter, freq, call, message, time};
}
