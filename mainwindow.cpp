#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QHash>
#include <QSettings>
#include <QSignalBlocker>
#include <QComboBox>
#include <QPushButton>
#include <QTimer>
#include <cmath>
#include <hamlib/rig.h>

static int estimateMorseDurationMs(const QString &text, int wpm)
{
    if (wpm <= 0) {
        return 0;
    }

    static const QHash<QChar, QByteArray> morseMap = {
        {'A', ".-"},    {'B', "-..."},  {'C', "-.-."}, {'D', "-.."},   {'E', "."},
        {'F', "..-."},  {'G', "--."},   {'H', "...."}, {'I', ".."},    {'J', ".---"},
        {'K', "-.-"},   {'L', ".-.."},  {'M', "--"},   {'N', "-."},    {'O', "---"},
        {'P', ".--."},  {'Q', "--.-"},  {'R', ".-."},  {'S', "..."},   {'T', "-"},
        {'U', "..-"},   {'V', "...-"},  {'W', ".--"},  {'X', "-..-"},  {'Y', "-.--"},
        {'Z', "--.."},
        {'0', "-----"}, {'1', ".----"}, {'2', "..---"},{'3', "...--"},{'4', "....-"},
        {'5', "....."}, {'6', "-...."}, {'7', "--..."},{'8', "---.."},{'9', "----."},
        {'/', "-..-."}
    };

    const int ditMs = 1200 / wpm;
    int units = 0;
    const QStringList words = text.toUpper().split(' ', Qt::SkipEmptyParts);
    for (int w = 0; w < words.size(); ++w) {
        const QString &word = words[w];
        for (int i = 0; i < word.size(); ++i) {
            const QByteArray pattern = morseMap.value(word[i]);
            if (pattern.isEmpty()) {
                continue;
            }
            for (int s = 0; s < pattern.size(); ++s) {
                units += (pattern[s] == '-') ? 3 : 1;
                if (s < pattern.size() - 1) {
                    units += 1;
                }
            }
            if (i < word.size() - 1) {
                units += 3;
            }
        }
        if (w < words.size() - 1) {
            units += 7;
        }
    }

    return units * ditMs + 200;
}

static float mapStrengthToS(int raw)
{
    if (raw > 0) {
        return static_cast<float>(raw);
    }

    static const float rawVals[] = {0.0f, -12.0f, -24.0f, -36.0f, -48.0f, -60.0f};
    static const float outVals[] = {9.0f, 7.0f, 5.0f, 3.0f, 1.0f, 0.0f};
    const int count = static_cast<int>(sizeof(rawVals) / sizeof(rawVals[0]));

    if (raw >= rawVals[0]) {
        return outVals[0];
    }
    if (raw <= rawVals[count - 1]) {
        return outVals[count - 1];
    }

    for (int i = 0; i < count - 1; ++i) {
        if (raw <= rawVals[i] && raw >= rawVals[i + 1]) {
            const float t = (raw - rawVals[i]) / (rawVals[i + 1] - rawVals[i]);
            return outVals[i] + t * (outVals[i + 1] - outVals[i]);
        }
    }

    return outVals[count - 1];
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , rig(RIG_MODEL_TS590S, "COM7")
{
    ui->setupUi(this);
    pttOffTimer = new QTimer(this);
    pttOffTimer->setSingleShot(true);
    connect(pttOffTimer, &QTimer::timeout, this, [this]() {
        if (manualTx) {
            return;
        }
        if (!rig.setPtt(rxVfo, false)) {
            qDebug() << "Hamlib rig_set_ptt (off) failed:" << rig.lastError();
            return;
        }
        setOnAir(false);
    });
    swrTimer = new QTimer(this);
    swrTimer->setInterval(1000);
    connect(swrTimer, &QTimer::timeout, this, &MainWindow::updateSWRLabel);
    sMeterTimer = new QTimer(this);
    sMeterTimer->setInterval(1000);
    connect(sMeterTimer, &QTimer::timeout, this, &MainWindow::updateSMeterLabel);
    powerTimer = new QTimer(this);
    powerTimer->setInterval(1000);
    connect(powerTimer, &QTimer::timeout, this, &MainWindow::updatePowerLabel);
    alcTimer = new QTimer(this);
    alcTimer->setInterval(1000);
    connect(alcTimer, &QTimer::timeout, this, &MainWindow::updateALCLabel);
    setOnAir(false);
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
    if (ui->sValueLabel) {
        ui->sValueLabel->setFixedWidth(80);
    }
    if (ui->onAirLabel) {
        ui->onAirLabel->setFixedWidth(70);
    }
    if (!rig.setMorseSpeed(rxVfo, morseWpm)) {
        qDebug() << "Hamlib rig_set_morse_speed failed:" << rig.lastError();
    }
    setOnAir(false);
    updateSMeterLabel();
    updateAGCLabel();
    updateVoxLabel();
    updateAntennaLabel();

    connect(ui->leftFrequency, &FrequencyLabel::valueChanged, this, &MainWindow::onLeftFrequencyChanged);
    connect(ui->rightFrequency, &FrequencyLabel::valueChanged, this, &MainWindow::onRightFrequencyChanged);
    connect(ui->splitButton, &QPushButton::toggled, this, &MainWindow::onSplitToggled);
    connect(ui->swapButton, &QPushButton::clicked, this, &MainWindow::onSwapButtonClicked);
    connect(ui->copyButton, &QPushButton::clicked, this, &MainWindow::onCopyButtonClicked);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::onSendButtonClicked);
    connect(ui->antToggleButton, &QPushButton::clicked, this, &MainWindow::onAntennaToggleClicked);
    connect(ui->modeButton, &QPushButton::clicked, this, &MainWindow::onModeButtonClicked);
    if (ui->powerLevelCombo) {
        ui->powerLevelCombo->setCurrentText("100 W");
        connect(ui->powerLevelCombo, &QComboBox::currentTextChanged, this, [this](const QString &text) {
            const QString trimmed = text.trimmed();
            bool ok = false;
            const float watts = trimmed.split(' ').first().toFloat(&ok);
            if (!ok || watts <= 0.0f) {
                return;
            }
            const float ratio = watts / 100.0f;
            if (!rig.setPower(rxVfo, ratio)) {
                qDebug() << "Hamlib rig_set_level (RFPOWER) failed:" << rig.lastError();
            }
        });
    }
    for (const auto &band : bandConfigs) {
        connect(band.button, &QPushButton::clicked, this, &MainWindow::onBandButtonClicked);
    }
    connect(&rig, &Rig::modeChanged, this, &MainWindow::updateModeLabel);
    connect(ui->og3zButton, &QPushButton::clicked, this, &MainWindow::onSendTextButtonClicked);
    connect(ui->fiveNnTuButton, &QPushButton::clicked, this, &MainWindow::onSendTextButtonClicked);
    connect(ui->rFiveNnTuButton, &QPushButton::clicked, this, &MainWindow::onSendTextButtonClicked);
    connect(ui->og3zTwiceButton, &QPushButton::clicked, this, &MainWindow::onSendTextButtonClicked);
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

void MainWindow::onSendButtonClicked()
{
    if (manualTx) {
        if (!rig.setPtt(rxVfo, false)) {
            qDebug() << "Hamlib rig_set_ptt (off) failed:" << rig.lastError();
            return;
        }
        manualTx = false;
        setOnAir(false);
        if (ui->sendButton) {
            ui->sendButton->setText("Send");
        }
        return;
    }

    if (pttOffTimer && pttOffTimer->isActive()) {
        pttOffTimer->stop();
    }
    if (!rig.setPtt(rxVfo, true)) {
        qDebug() << "Hamlib rig_set_ptt failed:" << rig.lastError();
        return;
    }
    manualTx = true;
    setOnAir(true);
    if (ui->sendButton) {
        ui->sendButton->setText("Stop");
    }
}

void MainWindow::onAntennaToggleClicked()
{
    ant_t ant = RIG_ANT_NONE;
    if (!rig.getAntenna(rxVfo, &ant)) {
        qDebug() << "Hamlib rig_get_ant failed:" << rig.lastError();
        return;
    }

    ant_t next = RIG_ANT_1;
    if (ant & RIG_ANT_1) {
        next = RIG_ANT_2;
    }

    if (!rig.setAntenna(rxVfo, next)) {
        qDebug() << "Hamlib rig_set_ant failed:" << rig.lastError();
        return;
    }

    updateAntennaLabel();
}

void MainWindow::onModeButtonClicked()
{
    rmode_t next = RIG_MODE_USB;
    switch (currentMode) {
    case RIG_MODE_USB:
        next = RIG_MODE_CW;
        break;
    case RIG_MODE_CW:
        next = RIG_MODE_FM;
        break;
    case RIG_MODE_FM:
    default:
        next = RIG_MODE_USB;
        break;
    }

    if (!rig.setMode(rxVfo, next)) {
        qDebug() << "Hamlib rig_set_mode (RX) failed:" << rig.lastError();
        return;
    }
    updateModeLabel(next);
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

void MainWindow::onSendTextButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (!button) {
        return;
    }
    const QString text = button->text().trimmed();
    if (text.isEmpty()) {
        return;
    }

    if (pttOffTimer && pttOffTimer->isActive()) {
        pttOffTimer->stop();
    }
    if (!rig.setPtt(rxVfo, true)) {
        qDebug() << "Hamlib rig_set_ptt failed:" << rig.lastError();
        return;
    }
    setOnAir(true);
    qApp->processEvents();

    if (!rig.sendMorse(rxVfo, text)) {
        qDebug() << "Hamlib rig_send_morse failed:" << rig.lastError();
        if (!rig.setPtt(rxVfo, false)) {
            qDebug() << "Hamlib rig_set_ptt (off) failed:" << rig.lastError();
            return;
        }
        setOnAir(false);
        return;
    }

    const int durationMs = estimateMorseDurationMs(text, morseWpm);
    if (pttOffTimer) {
        pttOffTimer->start(durationMs > 0 ? durationMs : 200);
    }
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
        {1800000, RIG_MODE_CW},
        {1840000, RIG_MODE_USB},
        {1840000, RIG_MODE_USB}, // TODO:
        {1800000, RIG_MODE_LSB}
    };
    static const BandStep steps3_5[4] = {
        {3501000, RIG_MODE_CW},
        {3573000, RIG_MODE_USB},
        {3595000, RIG_MODE_USB},
        {3600000, RIG_MODE_LSB}
    };
    static const BandStep steps7[4] = {
        {7001000, RIG_MODE_CW},
        {7074000, RIG_MODE_USB},
        {7047500, RIG_MODE_USB},
        {7100000, RIG_MODE_LSB}
    };
    static const BandStep steps10[4] = {
        {10000000, RIG_MODE_CW}, // TODO:
        {10136000, RIG_MODE_USB},
        {10140000, RIG_MODE_USB},
        {10000000, RIG_MODE_USB} // TODO:
    };
    static const BandStep steps14[4] = {
        {14001000, RIG_MODE_CW},
        {14074000, RIG_MODE_USB},
        {14080000, RIG_MODE_USB},
        {14200000, RIG_MODE_USB}
    };
    static const BandStep steps18[4] = {
        {18000000, RIG_MODE_CW}, // TODO:
        {18100000, RIG_MODE_USB},
        {18104000, RIG_MODE_USB},
        {18000000, RIG_MODE_USB}
    };
    static const BandStep steps21[4] = {
        {21001000, RIG_MODE_CW},
        {21074000, RIG_MODE_USB},
        {21140000, RIG_MODE_USB},
        {21200000, RIG_MODE_USB}
    };
    static const BandStep steps24[4] = {
        {24000000, RIG_MODE_CW},
        {24915000, RIG_MODE_USB},
        {24919000, RIG_MODE_USB},
        {24000000, RIG_MODE_USB}
    };
    static const BandStep steps28[4] = {
        {28000000, RIG_MODE_CW},
        {28074000, RIG_MODE_USB},
        {28180000, RIG_MODE_USB},
        {28000000, RIG_MODE_USB}
    };
    static const BandStep steps50[4] = {
        {50100000, RIG_MODE_CW},
        {50313000, RIG_MODE_USB},
        {50318000, RIG_MODE_USB},
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
    case RIG_MODE_FM:
        label = "FM";
        break;
    case RIG_MODE_USB:
    default:
        label = "USB";
        break;
    }

    currentMode = mode;
    ui->modeButton->setText(QString::fromLatin1(label));
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

void MainWindow::setOnAir(bool enabled)
{
    if (!ui || !ui->onAirLabel) {
        return;
    }
    ui->onAirLabel->show();
    if (enabled) {
        ui->onAirLabel->setText("On Air");
        ui->onAirLabel->setStyleSheet("color: white; background-color: red; padding: 2px 6px;");
        if (ui->sTextLabel) {
            ui->sTextLabel->setText("P");
        }
        if (swrTimer) {
            swrTimer->start();
        }
        if (sMeterTimer) {
            sMeterTimer->stop();
        }
        if (powerTimer) {
            powerTimer->start();
        }
        if (alcTimer) {
            alcTimer->start();
        }
        if (ui->sValueLabel) {
            ui->sValueLabel->setText("--");
        }
        updateSWRLabel();
        updatePowerLabel();
        updateALCLabel();
    } else {
        ui->onAirLabel->setText(QString());
        ui->onAirLabel->setStyleSheet(QString());
        if (ui->sTextLabel) {
            ui->sTextLabel->setText("S");
        }
        if (swrTimer) {
            swrTimer->stop();
        }
        if (sMeterTimer && rig.isOpen()) {
            sMeterTimer->start();
        } else if (sMeterTimer) {
            sMeterTimer->stop();
        }
        if (powerTimer) {
            powerTimer->stop();
        }
        if (alcTimer) {
            alcTimer->stop();
        }
        if (ui->swrLabel) {
            ui->swrLabel->setText("--");
        }
        if (ui->alcValueLabel) {
            ui->alcValueLabel->setText(QString());
        }
        updateSMeterLabel();
    }
    ui->onAirLabel->repaint();
}

void MainWindow::updateSWRLabel()
{
    if (!ui || !ui->swrLabel) {
        return;
    }
    float swr = 0.0f;
    if (!rig.getSWR(rxVfo, &swr)) {
        qDebug() << "Hamlib rig_get_level (SWR) failed:" << rig.lastError();
        ui->swrLabel->setText("--");
        return;
    }
    ui->swrLabel->setText(QString::number(swr, 'f', 2));
}

void MainWindow::updateSMeterLabel()
{
    if (!ui || !ui->sValueLabel) {
        return;
    }
    if (!rig.isOpen()) {
        ui->sValueLabel->setText("--");
        return;
    }
    int strength = 0;
    if (!rig.getStrength(rxVfo, &strength)) {
        qDebug() << "Hamlib rig_get_level (STRENGTH) failed:" << rig.lastError();
        ui->sValueLabel->setText("--");
        return;
    }
    const float mapped = mapStrengthToS(strength);
    const QString formatted = QString("%1 dB")
        .arg(static_cast<int>(std::round(mapped)));
    ui->sValueLabel->setText(formatted);
    updateAGCLabel();
    updateVoxLabel();
    updateAntennaLabel();
}

void MainWindow::updatePowerLabel()
{
    if (!ui || !ui->sValueLabel) {
        return;
    }
    if (!rig.isOpen()) {
        ui->sValueLabel->setText("--");
        return;
    }
    float power = 0.0f;
    if (!rig.getPower(rxVfo, &power)) {
        qDebug() << "Hamlib rig_get_level (RFPOWER) failed:" << rig.lastError();
        ui->sValueLabel->setText("--");
        return;
    }
    const QString formatted = QString("%1 W")
        .arg(QString::number(power, 'f', 1));
    ui->sValueLabel->setText(formatted);
}

void MainWindow::updateALCLabel()
{
    if (!ui || !ui->alcValueLabel) {
        return;
    }
    if (!rig.isOpen()) {
        ui->alcValueLabel->setText(QString());
        return;
    }
    float alc = 0.0f;
    if (!rig.getALC(rxVfo, &alc)) {
        qDebug() << "Hamlib rig_get_level (ALC) failed:" << rig.lastError();
        ui->alcValueLabel->setText(QString());
        return;
    }
    ui->alcValueLabel->setText(QString::number(alc, 'f', 1));
}

void MainWindow::updateAGCLabel()
{
    if (!ui || !ui->agcLabel) {
        return;
    }
    static bool warned = false;
    if (!rig.isOpen()) {
        ui->agcLabel->setText("AGC --");
        return;
    }
    int agc = 0;
    if (!rig.getAGC(rxVfo, &agc)) {
        if (!warned) {
            qDebug() << "Hamlib rig_get_level (AGC) failed:" << rig.lastError();
            warned = true;
        }
        ui->agcLabel->setText("AGC --");
        return;
    }
    warned = false;

    QString label = "AGC";
    switch (agc) {
    case RIG_AGC_OFF:
        label = "AGC NONE";
        break;
    case RIG_AGC_SLOW:
        label = "AGC SLOW";
        break;
    case RIG_AGC_FAST:
    case RIG_AGC_SUPERFAST:
    case 5:
        label = "AGC FAST";
        break;
    default:
        label = QString("AGC %1").arg(agc);
        break;
    }
    ui->agcLabel->setText(label);
}

void MainWindow::updateVoxLabel()
{
    if (!ui || !ui->voxLabel) {
        return;
    }
    if (!rig.isOpen()) {
        ui->voxLabel->setText(QString());
        return;
    }
    bool enabled = false;
    if (!rig.getVoxEnabled(rxVfo, &enabled)) {
        qDebug() << "Hamlib rig_get_func (VOX) failed:" << rig.lastError();
        ui->voxLabel->setText(QString());
        return;
    }
    ui->voxLabel->setText(enabled ? "VOX" : QString());
}

void MainWindow::updateAntennaLabel()
{
    if (!ui || !ui->antValueLabel) {
        return;
    }
    if (!rig.isOpen()) {
        ui->antValueLabel->setText("--");
        return;
    }

    ant_t ant = RIG_ANT_NONE;
    if (!rig.getAntenna(rxVfo, &ant)) {
        qDebug() << "Hamlib rig_get_ant failed:" << rig.lastError();
        ui->antValueLabel->setText("--");
        return;
    }

    QString value = "--";
    if (ant & RIG_ANT_1) {
        value = "1";
    } else if (ant & RIG_ANT_2) {
        value = "2";
    }
    ui->antValueLabel->setText(value);
}
