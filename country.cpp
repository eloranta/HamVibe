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
        QStringList prefixes;
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

        qDebug().noquote() << "CTY" << country << continent
                           << "prefixes" << prefixes.join(",")
                           << "calls" << calls.join(",");
    }
}

QString Country::GetCountry(const QString &call) const
{
    return call;
}
