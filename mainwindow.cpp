#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <hamlib/rig.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->leftFrequency->setPrefix('A');
    ui->leftFrequency->setValue(14000000);

    qDebug() << "hamlib version:" << hamlib_version;

    connect(ui->leftFrequency, &FrequencyLabel::valueChanged, this, [](int value, QChar prefix) {
        qDebug() << "Frequency value changed:" << prefix << value;
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
