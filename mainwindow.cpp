#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , rig(RIG_MODEL_TS590S, "COM7") // TODO: make configurable
{
    ui->setupUi(this);

    if (!rig.open()) {
        qDebug() << "Hamlib rig_open failed:" << rig.lastError();
        ui->meterLabel->setText("S-meter: -- dB");
        return;
    }

    pollTimer = new QTimer(this);
    pollTimer->setInterval(1000);
    connect(pollTimer, &QTimer::timeout, this, [this]() { poll(); });
    pollTimer->start();
    poll();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::poll()
{
    int value = 0;
    if (!rig.readSMeter(value)) {
        ui->meterLabel->setText("S-meter: -- dB");
        return;
    }

    ui->meterLabel->setText(QString("S-meter: %1 dB").arg(value));
}
