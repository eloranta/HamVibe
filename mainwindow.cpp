#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSignalBlocker>
#include <hamlib/rig.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , rig(RIG_MODEL_TS590S, "COM7")
{
    ui->setupUi(this);

    qDebug() << "hamlib version:" << hamlib_version;

    if (!rig.open()) {
        qDebug() << "Hamlib rig_open failed:" << rig.lastError();
        return;
    }

    if (!rig.getActiveVfo(&rxVfo)) {
        qDebug() << "Hamlib getActiveVfo failed:" << rig.lastError();
        return;
    }

    if (!rig.getSplit(rxVfo, &split, &txVfo)) { // txVfo TODO:
        qDebug() << "Hamlib get split failed:" << rig.lastError();
        return;
    }

    txVfo = (rxVfo == RIG_VFO_A) ? RIG_VFO_B : RIG_VFO_A;
    ui->splitButton->setCheckable(true);
    ui->splitButton->setChecked(split);

    int rxFreq = 0;
    if (rig.readFrequency(rxVfo, rxFreq)) {
        ui->leftFrequency->setPrefix(rxVfo == RIG_VFO_A ? 'A' : 'B');
        ui->leftFrequency->setValue(rxFreq);
    } else {
        qDebug() << "Hamlib rig_get_freq (RX) failed:" << rig.lastError();
    }
    int txFreq = 0;
    if (rig.readFrequency(txVfo, txFreq)) {
        qDebug() << txFreq;
        ui->rightFrequency->setPrefix(txVfo == RIG_VFO_A ? 'A' : 'B');
        ui->rightFrequency->setValue(txFreq);
    } else {
        qDebug() << "Hamlib rig_get_freq (TX) failed:" << rig.lastError();
    }

    if (split) {
        ui->rightFrequency->show();
     } else {
        ui->rightFrequency->hide();
    }

    connect(ui->leftFrequency, &FrequencyLabel::valueChanged, this, &MainWindow::onLeftFrequencyChanged);
    connect(ui->rightFrequency, &FrequencyLabel::valueChanged, this, &MainWindow::onRightFrequencyChanged);
    connect(ui->splitButton, &QPushButton::toggled, this, &MainWindow::onSplitToggled);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLeftFrequencyChanged(int value, QChar prefix)
{
    qDebug() << "Frequency value changed:" << prefix << value;
    rig.setFrequency(value);
}

void MainWindow::onRightFrequencyChanged(int value, QChar prefix)
{
    qDebug() << "Frequency value changed:" << prefix << value;
    rig.setFrequency(prefix == QChar('A') ? RIG_VFO_A : RIG_VFO_B, value);
}

void MainWindow::onSplitToggled(bool enabled)
{
    qDebug() << enabled;

    if (!rig.setSplit(rxVfo, txVfo, enabled)) {
        qDebug() << "Hamlib setSplit failed:" << rig.lastError();
        QSignalBlocker blocker(ui->splitButton);
        ui->splitButton->setChecked(!enabled);
        return;
    }

    if (enabled) {
        int txFreq = 0;
        if (rig.readFrequency(txVfo, txFreq)) {
            ui->rightFrequency->setPrefix(rxVfo == RIG_VFO_A ? 'B' : 'A');
            ui->rightFrequency->setValue(txFreq);
        } else {
            qDebug() << "Hamlib rig_get_freq (TX) failed:" << rig.lastError();
        }
        ui->rightFrequency->show();
    } else {
        ui->rightFrequency->hide();
    }
}
