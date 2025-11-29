#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QCoreApplication>
#include <QHeaderView>

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

    q.exec(
        "CREATE TABLE IF NOT EXISTS spots ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  spot TEXT"
        ");");
    // seed test data if empty
    q.exec("SELECT COUNT(*) FROM spots;");
    if (q.next() && q.value(0).toInt() == 0) {
        q.exec("INSERT INTO spots (spot) VALUES ('CQ DX DE TEST'), ('S9YY 14045 CW'), ('OH1AA 7020 SSB');");
    }

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

    auto *spotsModel = new QSqlTableModel(this, db);
    spotsModel->setTable("spots");
    spotsModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    spotsModel->select();
    spotsModel->setHeaderData(1, Qt::Horizontal, "Spot");

    ui->spotView->setModel(spotsModel);
    ui->spotView->hideColumn(0);
    ui->spotView->resizeColumnsToContents();
}

MainWindow::~MainWindow()
{
    delete ui;
}
