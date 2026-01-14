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

    connect(ui->leftFrequency, &FrequencyLabel::valueChanged, this, [](int value) {
        qDebug() << "Frequency value changed:" << value;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
