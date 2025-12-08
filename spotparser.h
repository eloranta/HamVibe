#pragma once

#include <QString>
#include <optional>

struct ParsedSpot
{
    QString spotter;
    QString freq;
    QString call;
    QString message;
    QString time;
};

std::optional<ParsedSpot> ParseInputString(const QString &line);
