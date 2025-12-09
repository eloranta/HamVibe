#include "calllookup.h"

#include <QRegularExpression>

QString findCountryForCall(const QString &call,
                           const QHash<QString, QString> &prefixToCountry)
{
    if (call.isEmpty() || prefixToCountry.isEmpty())
        return {};

    const QStringList parts = call.toUpper().split('/', Qt::SkipEmptyParts);
    QString primaryToken;
    for (const QString &p : parts)
    {
        if (p.contains(QRegularExpression("\\d")))
        {
            if (p.size() > primaryToken.size())
                primaryToken = p;
        }
    }

    QStringList candidates;
    if (!primaryToken.isEmpty())
        candidates << primaryToken;
    for (const QString &p : parts)
    {
        if (p != primaryToken)
            candidates << p;
    }
    if (candidates.isEmpty())
        candidates << call.toUpper();

    QString bestCountry;
    int bestLen = 0;
    for (const QString &candidate : candidates)
    {
        for (auto it = prefixToCountry.constBegin(); it != prefixToCountry.constEnd(); ++it)
        {
            const QString &prefix = it.key();
            if (candidate.startsWith(prefix, Qt::CaseInsensitive) && prefix.size() > bestLen)
            {
                bestLen = prefix.size();
                bestCountry = it.value();
            }
        }

        if (bestLen > 0)
            break;
    }

    return bestCountry.toUpper();
}
