#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QAction>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , rig(RIG_MODEL_TS590S, "COM7") // TODO: make configurable
{
    ui->setupUi(this);
    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

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

void MainWindow::showSettingsDialog()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Settings");

    QVBoxLayout layout(&dialog);
    QLabel label("Settings are not implemented yet.", &dialog);
    QDialogButtonBox buttons(QDialogButtonBox::Close, &dialog);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    layout.addWidget(&label);
    layout.addWidget(&buttons);

    dialog.exec();
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
