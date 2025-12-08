#include "spotparser.h"

#include <QDebug>
#include <QRegularExpression>
#include <QtGlobal>

std::optional<ParsedSpot> ParseInputString(const QString &line)
{
    const QString linePadded = line.leftJustified(80, ' ');
    const int colonIdx = linePadded.indexOf(':', 6);
    if (colonIdx < 0)
    {
        qDebug() << "Parse failed (no colon) for line:" << line;
        return std::nullopt;
    }

    const QString spotter = linePadded.mid(6, colonIdx - 6).trimmed();

    const int freqStart = colonIdx + 1;
    const int freqLen = qMax(0, 24 - freqStart);
    const QString freq = linePadded.mid(freqStart, freqLen).trimmed();

    static const int callStart = linePadded.indexOf(QRegularExpression("\\S"), 24);
    if (callStart < 0)
    {
        qDebug() << "Parse failed (no call) for line:" << line;
        return std::nullopt;
    }

    const QString call = linePadded.mid(callStart, 10).trimmed();
    const int messageStart = callStart + 10;
    const QString message = linePadded.mid(messageStart, 34).trimmed();
    const QString time = linePadded.mid(70, 4).trimmed();

    if (spotter.isEmpty() || freq.isEmpty() || call.isEmpty() || time.isEmpty())
    {
        qDebug() << "Parse failed (missing fields) for line:" << line;
        return std::nullopt;
    }

    return ParsedSpot{spotter, freq, call, message, time};
}
