#pragma once

#include <QHash>
#include <QString>

QString findCountryForCall(const QString &call,
                           const QHash<QString, QString> &prefixToCountry);
