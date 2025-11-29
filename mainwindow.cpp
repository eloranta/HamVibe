#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open())
    {
        QMessageBox::critical(this, "DB error", db.lastError().text());
        return;
    }

    QSqlQuery q(db);
    q.exec("CREATE TABLE items (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, value INTEGER);");
    q.exec("INSERT INTO items (name, value) VALUES ('Alpha', 1), ('Bravo', 2), ('Charlie', 3);");

    auto *model = new QSqlTableModel(this, db);
    model->setTable("items");
    model->select();
    ui->tableView->setModel(model);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->resizeColumnsToContents();
}

MainWindow::~MainWindow()
{
    delete ui;
}
