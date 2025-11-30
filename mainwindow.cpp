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
    spotsModel->setHeaderData(3, Qt::Horizontal, "Freq");
    spotsModel->setHeaderData(4, Qt::Horizontal, "Spotter");
    spotsModel->setHeaderData(5, Qt::Horizontal, "Message");

    ui->spotView->setModel(spotsModel);
    ui->spotView->resizeColumnsToContents();
    ui->spotView->setColumnWidth(2, ui->spotView->columnWidth(2) * 2);
    ui->spotView->setColumnWidth(3, ui->spotView->columnWidth(3) * 2);
    ui->spotView->setColumnWidth(4, ui->spotView->columnWidth(4) * 2);

    //setupSpotsSocket();
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
    qDebug() << "Spots data received:" << data;
    const QList<QByteArray> lines = data.split('\n');

    QSqlQuery insert(db);
    insert.prepare(
        "INSERT INTO spots ([time], [call], [freq], [spotter], [message]) "
        "VALUES (:time, :call, :freq, :spotter, :message)");

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

        insert.bindValue(":time", time);
        insert.bindValue(":call", call);
        insert.bindValue(":freq", freq);
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
        QString prefix;
        if (parts.size() > 8)
        {
            QStringList listParts;
            for (int idx = 8; idx < parts.size(); ++idx)
            {
                const QString token = parts.at(idx).trimmed();
                if (!token.isEmpty())
                    listParts << token;
            }
            prefix = listParts.join(' ');
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

            prefix += ' ' + contTrimmed;
            ++i;

            if (contTrimmed.endsWith(';'))
                break;
        }

        if (prefix.endsWith(';'))
            prefix.chop(1);

        QDebug dbg = qDebug().noquote();
        dbg << "CTY:" << country << cqZone << ituZone << continent << latitude << longitude << offset;
        if (!prefix.isEmpty())
            dbg << prefix;
    }
}
