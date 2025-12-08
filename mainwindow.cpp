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
#include <QCheckBox>
#include <cmath>

class SpotsFilterProxy : public QSortFilterProxyModel
{
public:
    explicit SpotsFilterProxy(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {}

    void setBands(const QSet<QString> &bands)
    {
        allowedBands = bands;
        invalidateFilter();
    }

    void setContinents(const QSet<QString> &continents)
    {
        allowedContinents = continents;
        invalidateFilter();
    }

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override
    {
        const QModelIndex bandIdx = sourceModel()->index(source_row, 4, source_parent); // band
        const QModelIndex continentIdx = sourceModel()->index(source_row, 8, source_parent); // continent

        const QString bandVal = sourceModel()->data(bandIdx).toString().toUpper();
        const QString contVal = sourceModel()->data(continentIdx).toString().toUpper();

        // Band filter: if none selected, allow all; otherwise require match.
        if (!allowedBands.isEmpty() && !allowedBands.contains(bandVal))
            return false;

        // Continent filter: if none selected, allow all; if continent unknown, allow; otherwise require match.
        if (!allowedContinents.isEmpty())
        {
            if (!contVal.isEmpty())
            {
                if (!allowedContinents.contains(contVal))
                    return false;
            }
        }

        return true;
    }

private:
    QSet<QString> allowedBands;
    QSet<QString> allowedContinents;
};

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
        "  mode TEXT,"
        "  spotter TEXT,"
        "  continent TEXT,"
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
    spotsModel->setHeaderData(6, Qt::Horizontal, "Mode");
    spotsModel->setHeaderData(7, Qt::Horizontal, "Spotter");
    spotsModel->setHeaderData(8, Qt::Horizontal, "Continent");
    spotsModel->setHeaderData(9, Qt::Horizontal, "Message");

    spotsProxy = new SpotsFilterProxy(this);
    spotsProxy->setSourceModel(spotsModel);
    ui->spotView->setModel(spotsProxy);
    ui->spotView->hideColumn(0); // hide id
    ui->spotView->resizeColumnsToContents();
    ui->spotView->hideColumn(8); // hide continent
    ui->spotView->setColumnWidth(2, ui->spotView->columnWidth(2) * 2); // Call
    ui->spotView->setColumnWidth(3, ui->spotView->columnWidth(3) * 2); // Country
    ui->spotView->setColumnWidth(4, ui->spotView->columnWidth(4) * 2);
    ui->spotView->setColumnWidth(5, ui->spotView->columnWidth(5) * 2);
    ui->spotView->setColumnWidth(6, ui->spotView->columnWidth(6) * 2);
    ui->spotView->setColumnWidth(7, ui->spotView->columnWidth(7) * 2);
    ui->spotView->setColumnWidth(8, ui->spotView->columnWidth(8) * 2);
    ui->spotView->setColumnWidth(9, ui->spotView->columnWidth(9) * 2);

    const QList<QCheckBox *> bandChecks = {
        ui->checkBox160, ui->checkBox80, ui->checkBox40, ui->checkBox30, ui->checkBox20,
        ui->checkBox17, ui->checkBox15, ui->checkBox12, ui->checkBox10, ui->checkBox6, ui->checkBox2
    };
    for (QCheckBox *cb : bandChecks)
        connect(cb, &QCheckBox::toggled, this, &MainWindow::updateBandFilter);

    const QList<QCheckBox *> continentChecks = {
        ui->checkBoxEU, ui->checkBoxAF, ui->checkBoxAS, ui->checkBoxOC, ui->checkBoxNA, ui->checkBoxSA
    };
    for (QCheckBox *cb : continentChecks)
        connect(cb, &QCheckBox::toggled, this, &MainWindow::updateBandFilter);

    updateBandFilter();

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
        "INSERT INTO spots ([time], [call], [country], [band], [freq], [mode], [spotter], [continent], [message]) "
        "VALUES (:time, :call, :country, :band, :freq, :mode, :spotter, :continent, :message)");

    bool inserted = false;
    for (const QByteArray &lineRaw : lines)
    {
        const QString rawLine = QString::fromUtf8(lineRaw);
        QString line = rawLine.trimmed();
        if (line.isEmpty())
            continue;
        if (!line.startsWith("DX de "))
            continue;
        qDebug().noquote() << line;
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
        QString continent;
        QString mode;

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
        } else {
            qDebug() << "Parse failed for line:" << line;
            continue;
        }

        QString country = findCountryForCall(call);
        if (country.isEmpty())
            country = "Unknown Country";
        const QString spotterCountry = findCountryForCall(spotter);
        if (!spotterCountry.isEmpty())
            continent = continentForCountry(spotterCountry);
        mode = detectMode(message);
        bool okFreq = false;
        const double freqVal = freq.toDouble(&okFreq);
        QString band = "0";
        int meters = 0;
        double freqMHzTrunc = 0.0;
        bool freqTruncValid = false;
        QString freqDisplay = freq;
        if (okFreq && freqVal > 0)
        {
            double freqMHzRaw = freqVal;
            if (freqMHzRaw > 1000.0)
                freqMHzRaw /= 1000.0; // assume kHz input
            freqMHzTrunc = std::floor(freqMHzRaw * 100.0) / 100.0;
            freqTruncValid = true;

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

            // Skip spots in the 5 MHz range (60m not supported).
            if (freqMHzTrunc >= 5.0 && freqMHzTrunc < 6.0)
                meters = 0;

            if (meters > 0)
                band = QString("%1m").arg(meters);
            else
                band.clear();
        }

        // Override mode to Data for known digital sub-bands.
        if (meters > 0 && okFreq && freqTruncValid)
        {
            struct DataRange { int meters; double minMhz; double maxMhz; };
            static const QVector<DataRange> dataRanges = {
                {160, 1.840, 1.840},
                {80, 3.573, 3.575},
                {40, 7.0475, 7.074},
                {30, 10.136, 10.140},
                {20, 14.074, 14.080},
                {17, 18.100, 18.104},
                {15, 21.074, 21.140},
                {12, 24.915, 24.919},
                {10, 28.074, 28.180},
                {6, 50.313, 50.318},
                {2, 144.170, 144.174},
            };
            for (const DataRange &dr : dataRanges)
            {
                if (dr.meters == meters && freqMHzTrunc >= dr.minMhz && freqMHzTrunc <= dr.maxMhz)
                {
                    mode = "Data";
                    break;
                }
            }

            if (mode.isEmpty())
            {
                struct PhoneRange { int meters; double minMhz; double maxMhz; };
                static const QVector<PhoneRange> phoneRanges = {
                    {160, 1.800, 2.000},
                    {80, 3.500, 4.000},
                    {40, 7.000, 7.300},
                    {30, 10.136, 10.140}, // narrow allowed phone ranges vary; keep minimal overlap
                    {20, 14.000, 14.350},
                    {17, 18.068, 18.168},
                    {15, 21.000, 21.450},
                    {12, 24.890, 24.990},
                    {10, 28.000, 29.700},
                    {6, 50.100, 50.300},
                    {2, 144.000, 148.000},
                };
                for (const PhoneRange &pr : phoneRanges)
                {
                    if (pr.meters == meters && freqMHzTrunc >= pr.minMhz && freqMHzTrunc <= pr.maxMhz)
                    {
                        mode = "Phone";
                        break;
                    }
                }
            }
        }

        if (mode.isEmpty())
            mode = "CW";

        if (meters > 0 && !country.isEmpty() && !isLogSlotEmpty(country, meters))
            continue;

        insert.bindValue(":time", time);
        insert.bindValue(":call", call);
        insert.bindValue(":country", country);
        insert.bindValue(":band", band);
        insert.bindValue(":freq", freqDisplay);
        insert.bindValue(":mode", mode);
        insert.bindValue(":spotter", spotter);
        insert.bindValue(":continent", continent);
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

void MainWindow::updateBandFilter()
{
    if (!spotsProxy)
        return;

    QSet<QString> bands;
    auto addBandIfChecked = [&bands](QCheckBox *cb) {
        if (cb && cb->isChecked())
            bands.insert((cb->text() + "m").toUpper());
    };

    addBandIfChecked(ui->checkBox160);
    addBandIfChecked(ui->checkBox80);
    addBandIfChecked(ui->checkBox40);
    addBandIfChecked(ui->checkBox30);
    addBandIfChecked(ui->checkBox20);
    addBandIfChecked(ui->checkBox17);
    addBandIfChecked(ui->checkBox15);
    addBandIfChecked(ui->checkBox12);
    addBandIfChecked(ui->checkBox10);
    addBandIfChecked(ui->checkBox6);
    addBandIfChecked(ui->checkBox2);

    QSet<QString> continents;
    auto addContIfChecked = [&continents](QCheckBox *cb) {
        if (cb && cb->isChecked())
            continents.insert(cb->text().toUpper());
    };

    addContIfChecked(ui->checkBoxEU);
    addContIfChecked(ui->checkBoxAF);
    addContIfChecked(ui->checkBoxAS);
    addContIfChecked(ui->checkBoxOC);
    addContIfChecked(ui->checkBoxNA);
    addContIfChecked(ui->checkBoxSA);

    spotsProxy->setBands(bands);
    spotsProxy->setContinents(continents);
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
    countryToContinent.clear();
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

        // Skip entities whose principal prefix is marked with '*' (deleted/invalid in cty.dat).
        const QString mainPrefix = parts.at(7).trimmed();
        if (mainPrefix.startsWith('*'))
            continue;

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
        if (!continent.isEmpty())
            countryToContinent.insert(country.toUpper(), continent.toUpper());

        //QDebug dbg = qDebug().noquote();
        //dbg << "CTY:" << country << cqZone << ituZone << continent << latitude << longitude << offset;
        //if (!prefixes.isEmpty())
        //    dbg << prefixes.join(',');
    }

    qDebug().noquote() << "Prefix map entries:" << prefixToCountry.size();
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

QString MainWindow::continentForCountry(const QString &country) const
{
    if (country.isEmpty())
        return {};
    return countryToContinent.value(country.toUpper());
}

QString MainWindow::detectMode(const QString &message) const
{
    const QString text = message.toUpper();

    static const QStringList digiKeywords = {
        "FT8", "FT4", "RTTY", "PSK", "JT65", "JT9", "FSK", "OLIVIA", "PACTOR",
        "PACKET", "PKT", "SSTV", "ROS", "MSK144", "JS8", "DIGI", "DATA"
    };
    for (const QString &k : digiKeywords)
    {
        if (text.contains(k))
            return "Data";
    }

    if (text.contains("CW"))
        return "CW";

    static const QStringList phoneKeywords = {"SSB", "USB", "LSB", "AM", "FM", "PHONE", "PH", "VOICE"};
    for (const QString &k : phoneKeywords)
    {
        if (text.contains(k))
            return "Phone";
    }

    return {};
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
