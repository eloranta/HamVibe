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
#include <algorithm>
#include <array>
#include <utility>

double MainWindow::interpolateSmeterDb(int value) const
{
    static constexpr std::array<std::pair<int, double>, 9> kSmeterMap = {{
        {0, 0.0},
        {3, 1.0},
        {6, 3.0},
        {9, 5.0},
        {12, 7.0},
        {15, 9.0},
        {20, 20.0},
        {25, 40.0},
        {30, 60.0},
    }};

    if (value <= kSmeterMap.front().first) {
        return kSmeterMap.front().second;
    }
    if (value >= kSmeterMap.back().first) {
        return kSmeterMap.back().second;
    }

    for (size_t i = 1; i < kSmeterMap.size(); ++i) {
        if (value <= kSmeterMap[i].first) {
            const int x0 = kSmeterMap[i - 1].first;
            const int x1 = kSmeterMap[i].first;
            const double y0 = kSmeterMap[i - 1].second;
            const double y1 = kSmeterMap[i].second;
            const double t = (value - x0) / static_cast<double>(x1 - x0);
            return y0 + t * (y1 - y0);
        }
    }

    return kSmeterMap.back().second;
}

static QString modeToText(rmode_t mode)
{
    switch (mode) {
    case RIG_MODE_LSB:
        return "LSB";
    case RIG_MODE_USB:
        return "USB";
    case RIG_MODE_CW:
        return "CW";
    case RIG_MODE_FM:
        return "FM";
    case RIG_MODE_AM:
        return "AM";
    case RIG_MODE_RTTY:
        return "RTTY";
    default:
        return "UNK";
    }
}

static QChar vfoToPrefix(vfo_t vfo)
{
    switch (vfo) {
    case RIG_VFO_A:
        return QChar('A');
    case RIG_VFO_B:
        return QChar('B');
    case RIG_VFO_MAIN:
        return QChar('M');
    case RIG_VFO_SUB:
        return QChar('S');
    case RIG_VFO_TX:
        return QChar('T');
    case RIG_VFO_RX:
        return QChar('R');
    default:
        return QChar('?');
    }
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    connect(ui->leftFrequency, &FrequencyLabel::valueChanged, this, [this](int value, QChar) {
        if (rig) {
            rig->setFrequency(value);
        }
    });
    connect(ui->rightFrequency, &FrequencyLabel::valueChanged, this, [this](int value, QChar) {
        if (rig) {
            rig->setFrequency(RIG_VFO_SUB, value);
        }
    });

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

}

void MainWindow::toggleLsbUsb()
{
    if (!rig) {
        return;
    }
    const bool nextLsb = !lsbSelected;
    const int mode = nextLsb ? RIG_MODE_LSB : RIG_MODE_USB;
    if (!rig->setMode(mode)) {
        return;
    }
    lsbSelected = nextLsb;
}

void MainWindow::setCwMode()
{
    if (!rig) {
        return;
    }
    rig->setMode(RIG_MODE_CW);
}

void MainWindow::toggleFmAm()
{
    if (!rig) {
        return;
    }
    const bool nextFm = !fmSelected;
    const int mode = nextFm ? RIG_MODE_FM : RIG_MODE_AM;
    if (!rig->setMode(mode)) {
        return;
    }
    fmSelected = nextFm;
}

void MainWindow::poll()
{
    static const QString kNormalChunkStyle =
        "QProgressBar::chunk {"
        " background-color: #000000;"
        " width: 3px;"
        " margin:1px;"
        "}";
    static const QString kAlertChunkStyle =
        "QProgressBar::chunk {"
        " background-color: #d00000;"
        " width: 3px;"
        " margin:1px;"
        "}";

    bool ptt = false;
    if (!rig->getPtt(ptt)) {
         return;
    }

    rmode_t mode = RIG_MODE_NONE;
    if (rig->readMode(mode)) {
        ui->modeLabel->setText(modeToText(mode));
    }
    int frequency = 0;
    if (rig->readFrequency(frequency)) {
        ui->leftFrequency->setValue(frequency);
    }
    vfo_t vfo = RIG_VFO_CURR;
    if (rig->readVfo(vfo)) {
        ui->leftFrequency->setPrefix(vfoToPrefix(vfo));
    }
    bool splitOn = false;
    vfo_t txVfo = RIG_VFO_CURR;
    if (rig->readSplit(splitOn, txVfo)) {
        ui->rightFrequency->setVisible(splitOn);
        if (splitOn) {
            int subFrequency = 0;
            if (rig->readFrequency(RIG_VFO_SUB, subFrequency)) {
                ui->rightFrequency->setValue(subFrequency);
                ui->rightFrequency->setPrefix(vfoToPrefix(RIG_VFO_SUB));
            }
        }
    }
    if (ptt) {
        ui->ptt->setText("On Air");
        ui->ptt->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(255, 0, 0);");

        ui->label->setEnabled(false);
        ui->sMeter->setEnabled(false);
        ui->sValue->setEnabled(false);
        ui->label_2->setEnabled(true);
        ui->powerMeter->setEnabled(true);
        ui->powerValue->setEnabled(true);
        ui->label_3->setEnabled(true);
        ui->alcMeter->setEnabled(true);
        ui->alcValue->setEnabled(true);
        ui->label_4->setEnabled(true);
        ui->swrMeter->setEnabled(true);
        ui->swrValue->setEnabled(true);

        ui->sMeter->setValue(0);
        ui->sValue->setText("0 dB");

        double value = 0;
        if (!rig->readPower(value)) {
            return;
        }
        ui->powerMeter->setValue(value);
        ui->powerValue->setText(QString("%1 W").arg(value, 0, 'f', 0));

        int alc = 0;
        if (rig->readAlc(alc)) {
            ui->alcMeter->setValue(alc);
            ui->alcValue->setText(QString::number(alc));
            ui->alcMeter->setStyleSheet(alc > 1 ? kAlertChunkStyle : kNormalChunkStyle);
        }

        int swr = 0;
        if (rig->readSwr(swr)) {
            const int swrDisplay = std::max(1, swr);
            ui->swrMeter->setValue(swrDisplay);
            ui->swrValue->setText(QString::number(swrDisplay));
            ui->swrMeter->setStyleSheet(swrDisplay > 2 ? kAlertChunkStyle : kNormalChunkStyle);
        }
    } else {
        ui->ptt->setText("Standby");
        ui->ptt->setStyleSheet("color: rgb(255, 255, 255); background-color: rgb(0, 0, 0);");

        ui->label->setEnabled(true);
        ui->sMeter->setEnabled(true);
        ui->sValue->setEnabled(true);
        ui->label_2->setEnabled(false);
        ui->powerMeter->setEnabled(false);
        ui->powerValue->setEnabled(false);
        ui->label_3->setEnabled(false);
        ui->alcMeter->setEnabled(false);
        ui->alcValue->setEnabled(false);
        ui->label_4->setEnabled(false);
        ui->swrMeter->setEnabled(false);
        ui->swrValue->setEnabled(false);

        ui->powerMeter->setValue(0);
        ui->powerValue->setText("0 W");
        ui->alcMeter->setValue(0);
        ui->alcValue->setText("");
        ui->swrMeter->setValue(1);
        ui->swrValue->setText("");
        ui->alcMeter->setStyleSheet(kNormalChunkStyle);
        ui->swrMeter->setStyleSheet(kNormalChunkStyle);

        int value = 0;
        if (!rig->readSMeter(value)) {
            return;
        }
        ui->sMeter->setValue(value);
        const double db = interpolateSmeterDb(value);
        ui->sValue->setText(QString("%1 dB").arg(db, 0, 'f', 0));
    }
}
