#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->leftFrequency->setPrefix('A');
    ui->leftFrequency->setValue(14000000);

    connect(ui->leftFrequency, &FrequencyLabel::valueChanged, this, [](int digitIndex, QChar prefix) {
        qDebug() << "Digit index clicked:" << prefix << digitIndex;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
