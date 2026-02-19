#include "country.h"

#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QRegularExpression>

void Country::init()
{
    QFile file("cty.dat");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open cty.dat";
        return;
    }

    QTextStream in(&file);
    const QString content = in.readAll();
    ParseCty(content);
}

void Country::ParseCty(const QString &content)
{
    const QStringList blocks = content.split(';', Qt::SkipEmptyParts);
    for (const QString &block : blocks) {
        const QString trimmedBlock = block.trimmed();
        if (trimmedBlock.isEmpty()) {
            continue;
        }

        const QStringList lines = trimmedBlock.split(QRegularExpression(R"(\r?\n)"), Qt::SkipEmptyParts);
        if (lines.isEmpty()) {
            continue;
        }

        const QString header = lines.first().trimmed();
        const QStringList headerParts = header.split(':', Qt::SkipEmptyParts);
        if (headerParts.size() < 8) {
            continue;
        }

        const QString country = headerParts.at(0).trimmed();
        const QString continent = headerParts.at(3).trimmed();
        const QString headerPrefix = headerParts.at(7).trimmed();
        if (headerPrefix.startsWith('*')) {
            continue;
        }
        QStringList prefixes;
        if (!headerPrefix.isEmpty()) {
            prefixes << headerPrefix;
        }

        QStringList calls;

        QString rest;
        for (int i = 1; i < lines.size(); ++i) {
            if (!rest.isEmpty()) {
                rest.append(',');
            }
            rest.append(lines.at(i));
        }

        const QStringList tokens = rest.split(',', Qt::SkipEmptyParts);
        for (QString token : tokens) {
            token = token.trimmed();
            if (token.isEmpty()) {
                continue;
            }
            const bool isCall = token.startsWith('=');
            if (isCall) {
                token.remove(0, 1);
            }
            token.replace(QRegularExpression(R"(\(.*?\))"), "");
            token.replace(QRegularExpression(R"(\[.*?\])"), "");
            token = token.trimmed();
            if (token.isEmpty()) {
                continue;
            }
            if (isCall) {
                calls << token;
            } else {
                prefixes << token;
            }
        }

        for (const QString &call : calls) {
            callMap.insert(call.toUpper(), country);
        }
        for (const QString &prefix : prefixes) {
            prefixMap.insert(prefix.toUpper(), country);
        }

        // qDebug().noquote() << "CTY" << country << continent
        //                    << "prefixes" << prefixes.join(",")
        //                    << "calls" << calls.join(",");
    }
}

QString Country::GetCountry(const QString &call) const
{
    const QString key = call.toUpper();

    auto resolve = [&](const QString &k) -> QString {
        const QString direct = callMap.value(k, QString());
        if (!direct.isEmpty()) {
            return direct;
        }
        QString best;
        for (auto it = prefixMap.constBegin(); it != prefixMap.constEnd(); ++it) {
            const QString &prefix = it.key();
            if (k.startsWith(prefix)) {
                if (prefix.size() > best.size()) {
                    best = prefix;
                }
            }
        }
        if (!best.isEmpty()) {
            return prefixMap.value(best);
        }
        return QString();
    };

    auto normalizeName = [](QString name) -> QString {
        const QString up = name.trimmed().toUpper();
        if (up == "UNITED STATES") return "UNITED STATES OF AMERICA";
        if (up == "FED. REP. OF GERMANY") return "FEDERAL REPUBLIC OF GERMANY";
        if (up == "VIETNAM") return "VIET NAM";
        if (up == "SOUTH AFRICA") return "REPUBLIC OF SOUTH AFRICA";
        if (up == "ST. BARTHELEMY") return "SAINT BARTHELEMY";
        if (up == "SEYCHELLES") return "SEYCHELLES ISLANDS";
        if (up == "ST. VINCENT") return "SAINT VINCENT";
        if (up == "ST. MARTIN") return "SAINT MARTIN";
        if (up == "ST. LUCIA") return "SAINT LUCIA";
        if (up == "ST. HELENA") return "SAINT HELENA";
        if (up == "ST. KITTS & NEVIS") return "SAINT KITTS & NEVIS";
        if (up == "MAURITIUS") return "MAURITIUS ISLAND";
        if (up == "FIJI") return "FIJI ISLANDS";
        if (up == "ASIATIC TURKEY") return "TURKEY";
        if (up == "SOV MIL ORDER OF MALTA") return "SOVEREIGN MILITARY ORDER OF MALTA";
        if (up == "BRUNEI DARUSSALAM") return "BRUNEI";

        return name;
    };

    QString result = resolve(key);
    if (!result.isEmpty()) {
        return normalizeName(result);
    }

    const QStringList parts = key.split('/', Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        result = resolve(part);
        if (!result.isEmpty()) {
            return normalizeName(result);
        }
    }
    return QString();
}
