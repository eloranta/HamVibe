#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , rig(RIG_MODEL_TS590S, "COM7") // TODO: make configurable
{
    ui->setupUi(this);

    if (!rig.open()) {
        qDebug() << "Hamlib rig_open failed:" << rig.lastError();
        return;
    }

    int freq = 0;
    if (!rig.readFrequency(freq)) {
        qDebug() << "Hamlib rig_open failed:" << rig.lastError();
        return;
    }

    qDebug() << freq;
}

MainWindow::~MainWindow()
{
    delete ui;
}
