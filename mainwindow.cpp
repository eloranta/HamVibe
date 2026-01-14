#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
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

    vfo_t activeVfo = RIG_VFO_NONE;
    if (!rig.getActiveVfo(&activeVfo)) {
        qDebug() << "Hamlib getActiveVfo failed:" << rig.lastError();
        return;
    }
    qDebug() << activeVfo;

    int rxFreq = 0;
    if (rig.readFrequency(activeVfo, rxFreq)) {
        qDebug() << rxFreq;
        ui->leftFrequency->setPrefix(activeVfo == RIG_VFO_A ? 'A' : 'B');
        ui->leftFrequency->setValue(rxFreq);
    } else {
        qDebug() << "Hamlib rig_get_freq (RX) failed:" << rig.lastError();
    }

    connect(ui->leftFrequency, &FrequencyLabel::valueChanged, this, [this](int value, QChar prefix) {
        qDebug() << "Frequency value changed:" << prefix << value;
        rig.setFrequency(value);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}
