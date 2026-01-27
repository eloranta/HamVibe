#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QAction>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QSettings>
#include <QSignalBlocker>
#include <QStyle>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::togglePtt);

    QSettings settings;
    const int model = settings.value("rig/model", RIG_MODEL_TS590S).toInt();
    const QString port = settings.value("rig/port", "COM7").toString();
    rig = std::make_unique<Rig>(static_cast<rig_model_t>(model), port);

    if (!rig->open()) {
        qDebug() << "Hamlib rig_open failed:" << rig->lastError();
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

void MainWindow::showSettingsDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Settings");

    QVBoxLayout layout(&dialog);
    QFormLayout form;
    QComboBox rigCombo(&dialog);
    rigCombo.addItem("RIG_MODEL_TS590S", RIG_MODEL_TS590S);
    rigCombo.addItem("RIG_MODEL_TS590SG", RIG_MODEL_TS590SG);

    QSettings settings;
    const int currentModel = settings.value("rig/model", RIG_MODEL_TS590S).toInt();
    const int index = rigCombo.findData(currentModel);
    rigCombo.setCurrentIndex(index >= 0 ? index : 0);

    form.addRow("Rig:", &rigCombo);

    QComboBox portCombo(&dialog);
    portCombo.addItem("COM7", "COM7");
    portCombo.addItem("COM 4", "COM 4");

    const QString currentPort = settings.value("rig/port", "COM7").toString();
    const int portIndex = portCombo.findData(currentPort);
    portCombo.setCurrentIndex(portIndex >= 0 ? portIndex : 0);

    form.addRow("Port:", &portCombo);

    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout.addLayout(&form);
    layout.addWidget(&buttons);

    if (dialog.exec() == QDialog::Accepted) {
        settings.setValue("rig/model", rigCombo.currentData().toInt());
        settings.setValue("rig/port", portCombo.currentData().toString());
    }
}

void MainWindow::showAboutDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("About HamVibe");

    QVBoxLayout layout(&dialog);
    QLabel label("HamVibe\n\nS-meter monitor using hamlib.", &dialog);
    label.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QDialogButtonBox buttons(QDialogButtonBox::Close, &dialog);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout.addWidget(&label);
    layout.addWidget(&buttons);

    dialog.exec();
}

void MainWindow::togglePtt()
{
    if (!rig) {
        return;
    }
    bool ptt = false;
    if (!rig->getPtt(ptt)) {
        return;
    }
    ptt = !ptt;
    if (!rig->setPtt(ptt)) {
        return;
    }
    if (ptt) {
        ui->unitLabel->setText("5-10-25---50---75-100W");
        ui->meterBar->setMaximum(100);
    } else {
        ui->unitLabel->setText("--1-3-5-7--9--20-40-60");
        ui->meterBar->setMaximum(30);
    }
    poll();
}

void MainWindow::poll()
{
    bool ptt = false;
    if (!rig->getPtt(ptt)) {
         return;
    }
    if (ptt) {
        ui->busyLabel->setText("ON AIR");
        ui->busyLabel->setStyleSheet("QLabel { color: white; background-color: #b00020; padding: 2px 6px; }");
        double power = 0.0;
        if (!rig->readPower(power)) {
             return;
        }
        ui->meterBar->setValue(power);
    } else {
        ui->busyLabel->setText("BUSY");
        ui->busyLabel->setStyleSheet("QLabel { color: white; background-color: black; padding: 2px 6px; }");
        int value = 0;
        if (!rig->readSMeter(value)) {
             return;
        }
        ui->meterBar->setValue(value);
    }
}
