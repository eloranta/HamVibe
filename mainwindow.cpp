#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSettings>
#include <QSignalBlocker>
#include <QPushButton>
#include <hamlib/rig.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , rig(RIG_MODEL_TS590S, "COM7")
{
    ui->setupUi(this);
    initBandConfigs();
    loadBandSettings();

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
    rmode_t initialMode = RIG_MODE_NONE;
    if (rig.getMode(rxVfo, &initialMode, nullptr)) {
        updateModeLabel(initialMode);
    }

    connect(ui->leftFrequency, &FrequencyLabel::valueChanged, this, &MainWindow::onLeftFrequencyChanged);
    connect(ui->rightFrequency, &FrequencyLabel::valueChanged, this, &MainWindow::onRightFrequencyChanged);
    connect(ui->splitButton, &QPushButton::toggled, this, &MainWindow::onSplitToggled);
    connect(ui->swapButton, &QPushButton::clicked, this, &MainWindow::onSwapButtonClicked);
    connect(ui->copyButton, &QPushButton::clicked, this, &MainWindow::onCopyButtonClicked);
    for (const auto &band : bandConfigs) {
        connect(band.button, &QPushButton::clicked, this, &MainWindow::onBandButtonClicked);
    }
    connect(&rig, &Rig::modeChanged, this, &MainWindow::updateModeLabel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onLeftFrequencyChanged(int value, QChar prefix)
{
    qDebug() << "Frequency value changed:" << prefix << value;
    rig.setFrequency(value);
    if (currentBandIndex >= 0 && currentBandIndex < bandConfigs.size()) {
        const int level = bandLevels[currentBandIndex];
        if (level >= 0 && level < 4) {
            bandSavedFreqs[currentBandIndex][level] = value;
            saveBandFrequency(currentBandIndex, level, value);
        }
    }
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

void MainWindow::onSwapButtonClicked()
{
    vfo_t activeVfo = RIG_VFO_NONE;
    if (!rig.getActiveVfo(&activeVfo)) {
        qDebug() << "Hamlib rig_get_active_vfo failed:" << rig.lastError();
        return;
    }
    QChar leftPrefix;
    QChar rightPrefix;
    if (activeVfo == RIG_VFO_A)
    {
        activeVfo = RIG_VFO_B;
        leftPrefix = 'B';
        rightPrefix = 'A';
    }
    else
    {
        activeVfo = RIG_VFO_A;
        leftPrefix = 'A';
        rightPrefix = 'B';
    }
    if (!rig.setActiveVfo(activeVfo)) {
        qDebug() << "Hamlib rig_get_active_vfo failed:" << rig.lastError();
        return;
    }

    int freq = 0;

     if (!rig.readFrequency(activeVfo, freq)) {
        qDebug() << "Hamlib rig_get_freq failed:" << rig.lastError();
        return;
    }
    ui->leftFrequency->setPrefix(leftPrefix);
    ui->leftFrequency->setValue(freq);

    if (!rig.readFrequency(activeVfo == RIG_VFO_A ? RIG_VFO_B : RIG_VFO_A, freq)) {
        qDebug() << "Hamlib rig_get_freq (B) failed:" << rig.lastError();
        return;
    }
    ui->rightFrequency->setPrefix(rightPrefix);
    ui->rightFrequency->setValue(freq);
}

void MainWindow::onCopyButtonClicked()
{
    vfo_t mainVfo = RIG_VFO_NONE;
    if (!rig.getActiveVfo(&mainVfo)) {
        qDebug() << "Hamlib rig_get_active_vfo failed:" << rig.lastError();
        return;
    }

    vfo_t subVfo = RIG_VFO_NONE;
    if (mainVfo == RIG_VFO_A)
        subVfo = RIG_VFO_B;
    else
        subVfo = RIG_VFO_A;

    int freq = 0;
    if (!rig.readFrequency(mainVfo, freq)) {
        qDebug() << "Hamlib rig_get_freq (RX) failed:" << rig.lastError();
        return;
    }
    if (!rig.setFrequency(subVfo, freq)) {
        qDebug() << "Hamlib rig_set_freq (TX) failed:" << rig.lastError();
        return;
    }

    ui->rightFrequency->setValue(freq);
}

void MainWindow::onBandButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button) {
        return;
    }

    int bandIndex = -1;
    for (int i = 0; i < bandConfigs.size(); ++i) {
        if (bandConfigs[i].button == button) {
            bandIndex = i;
            break;
        }
    }
    if (bandIndex < 0) {
        return;
    }

    currentBandIndex = bandIndex;
    setSelectedBandButton(button);

    int &level = bandLevels[bandIndex];
    if (level < 0 || level >= 3) {
        level = 0;
    } else {
        level = (level + 1) % 4;
    }

    int nextFreq = bandConfigs[bandIndex].steps[level].freq;
    if (bandSavedFreqs[bandIndex][level] > 0) {
        nextFreq = bandSavedFreqs[bandIndex][level];
    }

    if (!rig.setFrequency(rxVfo, nextFreq)) {
        qDebug() << "Hamlib rig_set_freq (RX) failed:" << rig.lastError();
        return;
    }
    if (!rig.setMode(rxVfo, bandConfigs[bandIndex].steps[level].mode)) {
        qDebug() << "Hamlib rig_set_mode (RX) failed:" << rig.lastError();
        return;
    }

    ui->leftFrequency->setValue(nextFreq);
}

void MainWindow::initBandConfigs()
{
    bandConfigs.clear();
    bandConfigs.reserve(10);

    auto addBand = [this](const char *key, QPushButton *button, const BandStep (&steps)[4]) {
        BandConfig config{};
        config.key = key;
        config.button = button;
        for (int i = 0; i < 4; ++i) {
            config.steps[i] = steps[i];
        }
        bandConfigs.push_back(config);
    };

    static const BandStep steps1_8[4] = {
        {1840000, RIG_MODE_USB},
        {1840000, RIG_MODE_USB}, // TODO:
        {1800000, RIG_MODE_CW},
        {1800000, RIG_MODE_LSB}
    };
    static const BandStep steps3_5[4] = {
        {3573000, RIG_MODE_USB},
        {3595000, RIG_MODE_USB},
        {3501000, RIG_MODE_CW},
        {3600000, RIG_MODE_LSB}
    };
    static const BandStep steps7[4] = {
        {7074000, RIG_MODE_USB},
        {7047500, RIG_MODE_USB},
        {7001000, RIG_MODE_CW},
        {7100000, RIG_MODE_LSB}
    };
    static const BandStep steps10[4] = {
        {10136000, RIG_MODE_USB},
        {10140000, RIG_MODE_USB},
        {10000000, RIG_MODE_CW}, // TODO:
        {10000000, RIG_MODE_USB} // TODO:
    };
    static const BandStep steps14[4] = {
        {14074000, RIG_MODE_USB},
        {14080000, RIG_MODE_USB},
        {14001000, RIG_MODE_CW},
        {14200000, RIG_MODE_USB}
    };
    static const BandStep steps18[4] = {
        {18100000, RIG_MODE_USB},
        {18104000, RIG_MODE_USB},
        {18000000, RIG_MODE_CW}, // TODO:
        {18000000, RIG_MODE_USB}
    };
    static const BandStep steps21[4] = {
        {21074000, RIG_MODE_USB},
        {21140000, RIG_MODE_USB},
        {21001000, RIG_MODE_CW},
        {21200000, RIG_MODE_USB}
    };
    static const BandStep steps24[4] = {
        {24915000, RIG_MODE_USB},
        {24919000, RIG_MODE_USB},
        {24000000, RIG_MODE_CW},
        {24000000, RIG_MODE_USB}
    };
    static const BandStep steps28[4] = {
        {28074000, RIG_MODE_USB},
        {28180000, RIG_MODE_USB},
        {28000000, RIG_MODE_CW},
        {28000000, RIG_MODE_USB}
    };
    static const BandStep steps50[4] = {
        {50313000, RIG_MODE_USB},
        {50318000, RIG_MODE_USB},
        {50100000, RIG_MODE_CW},
        {50300000, RIG_MODE_USB}
    };

    addBand("1.8", ui->band1_8, steps1_8);
    addBand("3.5", ui->pushButton3_5, steps3_5);
    addBand("7", ui->pushButton7, steps7);
    addBand("10", ui->pushButton10, steps10);
    addBand("14", ui->pushButton14, steps14);
    addBand("18", ui->pushButton18, steps18);
    addBand("21", ui->pushButton21, steps21);
    addBand("24", ui->pushButton24, steps24);
    addBand("28", ui->pushButton28, steps28);
    addBand("50", ui->pushButton50, steps50);
}

void MainWindow::loadBandSettings()
{
    QSettings settings("HamVibe", "HamVibe");
    for (int i = 0; i < bandConfigs.size(); ++i) {
        for (int level = 0; level < 4; ++level) {
            const QString key = QString("band/%1/level%2").arg(bandConfigs[i].key).arg(level + 1);
            bandSavedFreqs[i][level] = settings.value(key, 0).toInt();
        }
    }
}

void MainWindow::saveBandFrequency(int bandIndex, int level, int frequency)
{
    QSettings settings("HamVibe", "HamVibe");
    if (bandIndex < 0 || bandIndex >= bandConfigs.size()) {
        return;
    }
    const QString key = QString("band/%1/level%2").arg(bandConfigs[bandIndex].key).arg(level + 1);
    settings.setValue(key, frequency);
}

void MainWindow::updateModeLabel(rmode_t mode)
{
    const char *label = "USB";
    switch (mode) {
    case RIG_MODE_LSB:
        label = "LSB";
        break;
    case RIG_MODE_CW:
        label = "CW";
        break;
    case RIG_MODE_USB:
    default:
        label = "USB";
        break;
    }

    ui->modeLabel->setText(QString::fromLatin1(label));
}

void MainWindow::setSelectedBandButton(QPushButton *button)
{
    if (selectedBandButton == button) {
        return;
    }

    if (selectedBandButton) {
        selectedBandButton->setStyleSheet(QString());
    }

    selectedBandButton = button;
    if (selectedBandButton) {
        selectedBandButton->setStyleSheet("QPushButton { border: 2px solid #1e88e5; }");
    }
}
