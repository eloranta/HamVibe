#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QCoreApplication>
#include <QHeaderView>
#include <QDebug>
#include <QRegularExpression>
#include <QFile>
#include <QTextStream>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(QCoreApplication::applicationDirPath() + "/hamvibe.db");
    if (!db.open())
    {
        QMessageBox::critical(this, "DB error", db.lastError().text());
        return;
    }

    QSqlQuery q(db);
    q.exec(
        "CREATE TABLE IF NOT EXISTS items ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  Prefix TEXT,"
        "  Entity TEXT,"
        "  Mix TEXT,"
        "  Ph TEXT,"
        "  CW TEXT,"
        "  RT TEXT,"
        "  SAT TEXT,"
        "  [160] TEXT,"
        "  [80] TEXT,"
        "  [40] TEXT,"
        "  [30] TEXT,"
        "  [20] TEXT,"
        "  [17] TEXT,"
        "  [15] TEXT,"
        "  [12] TEXT,"
        "  [10] TEXT,"
        "  [6] TEXT,"
        "  [2] TEXT"
        ");");

    q.exec("DROP TABLE IF EXISTS spots;");
    q.exec(
        "CREATE TABLE spots ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  time TEXT,"
        "  call TEXT,"
        "  country TEXT,"
        "  band TEXT,"
        "  freq TEXT,"
        "  spotter TEXT,"
        "  message TEXT"
        ");");
    q.exec("DELETE FROM spots;");

    auto *model = new QSqlTableModel(this, db);
    model->setTable("items");
    model->setEditStrategy(QSqlTableModel::OnFieldChange);
    model->select();

    model->setHeaderData(1, Qt::Horizontal, "Prefix");
    model->setHeaderData(2, Qt::Horizontal, "Entity");
    model->setHeaderData(3, Qt::Horizontal, "Mix");
    model->setHeaderData(4, Qt::Horizontal, "Ph");
    model->setHeaderData(5, Qt::Horizontal, "CW");
    model->setHeaderData(6, Qt::Horizontal, "RT");
    model->setHeaderData(7, Qt::Horizontal, "SAT");
    model->setHeaderData(8, Qt::Horizontal, "160");
    model->setHeaderData(9, Qt::Horizontal, "80");
    model->setHeaderData(10, Qt::Horizontal, "40");
    model->setHeaderData(11, Qt::Horizontal, "30");
    model->setHeaderData(12, Qt::Horizontal, "20");
    model->setHeaderData(13, Qt::Horizontal, "17");
    model->setHeaderData(14, Qt::Horizontal, "15");
    model->setHeaderData(15, Qt::Horizontal, "12");
    model->setHeaderData(16, Qt::Horizontal, "10");
    model->setHeaderData(17, Qt::Horizontal, "6");
    model->setHeaderData(18, Qt::Horizontal, "2");

    ui->tableView->setModel(model);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->hideColumn(0);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->sortByColumn(1, Qt::AscendingOrder);

    spotsModel = new QSqlTableModel(this, db);
    spotsModel->setTable("spots");
    spotsModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    spotsModel->select();
    while (spotsModel->canFetchMore())
        spotsModel->fetchMore();
    spotsModel->setHeaderData(1, Qt::Horizontal, "Time");
    spotsModel->setHeaderData(2, Qt::Horizontal, "Call");
    spotsModel->setHeaderData(3, Qt::Horizontal, "Country");
    spotsModel->setHeaderData(4, Qt::Horizontal, "Band");
    spotsModel->setHeaderData(5, Qt::Horizontal, "Freq");
    spotsModel->setHeaderData(6, Qt::Horizontal, "Spotter");
    spotsModel->setHeaderData(7, Qt::Horizontal, "Message");

    ui->spotView->setModel(spotsModel);
    ui->spotView->resizeColumnsToContents();
    ui->spotView->setColumnWidth(2, ui->spotView->columnWidth(2) * 2); // Call
    ui->spotView->setColumnWidth(3, ui->spotView->columnWidth(3) * 2); // Country
    ui->spotView->setColumnWidth(4, ui->spotView->columnWidth(4) * 2);
    ui->spotView->setColumnWidth(5, ui->spotView->columnWidth(5) * 2);
    ui->spotView->setColumnWidth(6, ui->spotView->columnWidth(6) * 2);
    ui->spotView->setColumnWidth(7, ui->spotView->columnWidth(7) * 2);

    setupSpotsSocket();
    parseCtyFile();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupSpotsSocket()
{
    spotSocket = new QTcpSocket(this);
    connect(spotSocket, &QTcpSocket::connected, this, &MainWindow::onSpotsConnected);
    connect(spotSocket, &QTcpSocket::readyRead, this, &MainWindow::onSpotsReadyRead);
    spotSocket->connectToHost("ham.connect.fi", 7300);
}

void MainWindow::onSpotsConnected()
{
    spotSocket->write("oh2lhe\n");
}

void MainWindow::onSpotsReadyRead()
{
    if (!spotsModel)
        return;

    const QByteArray data = spotSocket->readAll();
    // qDebug() << "Spots data received:" << data;
    const QList<QByteArray> lines = data.split('\n');

    QSqlQuery insert(db);
    insert.prepare(
        "INSERT INTO spots ([time], [call], [country], [band], [freq], [spotter], [message]) "
        "VALUES (:time, :call, :country, :band, :freq, :spotter, :message)");

    bool inserted = false;
    for (const QByteArray &lineRaw : lines)
    {
        const QString line = QString::fromUtf8(lineRaw).trimmed();
        if (line.isEmpty())
            continue;
        if (!line.startsWith("DX de "))
            continue;

        // Parse fields by fixed columns:
        // spotter: cols 7-? until ':'
        // freq: next, ends by col 24
        // call: starts col 29, ends col 38
        // message: cols 30-69
        // time: col 71 onwards (4 digits)
        QString spotter;
        QString freq;
        QString call;
        QString message;
        QString time;

        // Use regex to capture components with spaces preserved as in the example
        QRegularExpression re(
            R"(DX de\s+(\S+):\s+(\S+)\s+(\S+)\s+(.{0,39}?)\s+(\d{4})Z?)");
        QRegularExpressionMatch m = re.match(line);
        if (m.hasMatch()) {
            spotter = m.captured(1);
            freq = m.captured(2);
            call = m.captured(3);
            message = m.captured(4).trimmed();
            time = m.captured(5);
            qDebug() << "Parsed:" << spotter << freq << call << message << time;
        } else {
            qDebug() << "Parse failed for line:" << line;
            continue;
        }

        QString country = findCountryForCall(call);
        if (country.isEmpty())
            country = "Unknown Country";
        bool okFreq = false;
        const double freqVal = freq.toDouble(&okFreq);
        QString band = "0";
        int meters = 0;
        QString freqDisplay = freq;
        if (okFreq && freqVal > 0)
        {
            double freqMHzRaw = freqVal;
            if (freqMHzRaw > 1000.0)
                freqMHzRaw /= 1000.0; // assume kHz input
            const double freqMHzTrunc = std::floor(freqMHzRaw * 100.0) / 100.0;

            struct BandMap { double mhz; int meters; };
            static const QVector<BandMap> bandMap = {
                {144.0, 2},
                {50.0, 6},
                {28.0, 10},
                {24.0, 12},
                {21.0, 15},
                {18.0, 17},
                {14.0, 20},
                {10.0, 30},
                {7.0, 40},
                {3.5, 80},
                {1.6, 160},
            };
            for (const BandMap &entry : bandMap)
            {
                if (freqMHzTrunc >= entry.mhz)
                {
                    meters = entry.meters;
                    break;
                }
            }

            if (meters > 0)
                band = QString("%1m").arg(meters);
            else
                band.clear();
        }

        if (meters > 0 && !country.isEmpty() && !isLogSlotEmpty(country, meters))
            continue;

        insert.bindValue(":time", time);
        insert.bindValue(":call", call);
        insert.bindValue(":country", country);
        insert.bindValue(":band", band);
        insert.bindValue(":freq", freqDisplay);
        insert.bindValue(":spotter", spotter);
        insert.bindValue(":message", message);
        if (insert.exec())
            inserted = true;
        else
            qDebug() << "Insert failed:" << insert.lastError().text();
    }

    if (inserted)
    {
        spotsModel->select();
        while (spotsModel->canFetchMore())
            spotsModel->fetchMore();
        ui->spotView->scrollToBottom();
    }
}

void MainWindow::parseCtyFile()
{
    const QString path = QCoreApplication::applicationDirPath() + "/cty.dat";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "cty.dat open failed:" << path;
        return;
    }

    prefixToCountry.clear();
    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd())
        lines << in.readLine();

    int i = 0;
    while (i < lines.size())
    {
        QString line = lines.at(i++);
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty() || trimmed.startsWith('#'))
            continue;

        // Records start at column 1; following indented lines extend the prefix list.
        if (line.at(0).isSpace())
            continue;

        QStringList parts = line.split(':', Qt::KeepEmptyParts);
        if (parts.size() < 8)
            continue;

        QString country = parts.at(0).trimmed();
        QString cqZone = parts.at(1).trimmed();
        QString ituZone = parts.at(2).trimmed();
        QString continent = parts.at(3).trimmed();
        QString latitude = parts.at(4).trimmed();
        QString longitude = parts.at(5).trimmed();
        QString offset = parts.at(6).trimmed();

        // Do not include the main prefix (parts[7]); gather any list starting after it.
        QStringList prefixes;
        auto addTokens = [&prefixes](const QString &segment) {
            const QString cleaned = segment.endsWith(';') ? segment.left(segment.size() - 1) : segment;
            for (const QString &rawToken : cleaned.split(',', Qt::SkipEmptyParts))
            {
                QString token = rawToken.trimmed();
                if (token.isEmpty())
                    continue;

                // Drop any bracketed numeric suffixes like [31] or (31), including repeats
                static const QRegularExpression suffixRe(R"(\s*(\[\d+\]|\(\d+\))\s*)");
                token.remove(suffixRe);
                token = token.trimmed();
                if (token.isEmpty())
                    continue;

                if (token.at(0).isLetterOrNumber())
                    prefixes << token;
            }
        };

        if (parts.size() > 8)
        {
            for (int idx = 8; idx < parts.size(); ++idx)
                addTokens(parts.at(idx));
        }

        // Pull in continuation lines (comma-separated prefixes ending with ';').
        while (i < lines.size())
        {
            const QString cont = lines.at(i);
            const QString contTrimmed = cont.trimmed();
            if (contTrimmed.isEmpty() || contTrimmed.startsWith('#'))
            {
                ++i;
                continue;
            }
            if (!cont.isEmpty() && !cont.at(0).isSpace())
                break;

            addTokens(contTrimmed);
            ++i;

            if (contTrimmed.endsWith(';'))
                break;
        }

        for (const QString &p : prefixes)
            prefixToCountry.insert(p.toUpper(), country);

        //QDebug dbg = qDebug().noquote();
        //dbg << "CTY:" << country << cqZone << ituZone << continent << latitude << longitude << offset;
        //if (!prefixes.isEmpty())
        //    dbg << prefixes.join(',');
    }

    qDebug().noquote() << "Prefix map entries:" << prefixToCountry.size();
    for (auto it = prefixToCountry.constBegin(); it != prefixToCountry.constEnd(); ++it)
        qDebug().noquote() << "MAP" << it.key() << ":" << it.value();
}

QString MainWindow::findCountryForCall(const QString &call) const
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

bool MainWindow::isLogSlotEmpty(const QString &country, int meters) const
{
    static const QSet<int> allowedBands = {160, 80, 40, 30, 20, 17, 15, 12, 10, 6, 2};
    if (!allowedBands.contains(meters))
        return true;

    const QString column = QString("[%1]").arg(meters);
    QSqlQuery q(db);
    q.prepare(QString("SELECT %1 FROM items WHERE UPPER(Entity) = UPPER(:country) LIMIT 1").arg(column));
    q.bindValue(":country", country);
    if (q.exec() && q.next())
    {
        const QString val = q.value(0).toString().trimmed();
        return val.isEmpty();
    }

    // Try a fallback: match by prefix mapped to the same country
    QString altPrefix;
    for (auto it = prefixToCountry.constBegin(); it != prefixToCountry.constEnd(); ++it)
    {
        if (it.value().compare(country, Qt::CaseInsensitive) == 0)
        {
            altPrefix = it.key();
            break;
        }
    }
    if (!altPrefix.isEmpty())
    {
        QSqlQuery q2(db);
        q2.prepare(QString("SELECT %1 FROM items WHERE UPPER(Prefix) = UPPER(:prefix) LIMIT 1").arg(column));
        q2.bindValue(":prefix", altPrefix);
        if (q2.exec() && q2.next())
        {
            const QString val = q2.value(0).toString().trimmed();
            return val.isEmpty();
        }
    }

    return true;
}
