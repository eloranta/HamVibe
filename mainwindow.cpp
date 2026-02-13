#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "udpreceiver.h"
#include "delegate.h"

#include <QAction>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QSignalBlocker>
#include <QStyle>
#include <QVBoxLayout>
#include <algorithm>
#include <array>
#include <utility>

#include <QSqlTableModel>

#include <QTcpSocket>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

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

static QString bandFromFrequencyText(const QString &freqText)
{
    bool ok = false;
    const double value = freqText.toDouble(&ok);
    if (!ok || value <= 0.0) {
        return QString();
    }

    double mhz = value;
    if (mhz > 1000.0) {
        mhz /= 1000.0;
    }

    if (mhz >= 3.5 && mhz < 4.0) return "80";
    if (mhz >= 7.0 && mhz < 7.3) return "40";
    if (mhz >= 10.1 && mhz < 10.15) return "30";
    if (mhz >= 14.0 && mhz < 14.35) return "20";
    if (mhz >= 18.068 && mhz < 18.168) return "17";
    if (mhz >= 21.0 && mhz < 21.45) return "15";
    if (mhz >= 24.89 && mhz < 24.99) return "12";
    if (mhz >= 28.0 && mhz < 29.7) return "10";
    return QString();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const int defaultCwSpeed = 30;
    const int speedIndex = ui->morseSpeed->findText(QString::number(defaultCwSpeed));
    ui->morseSpeed->setCurrentIndex(speedIndex >= 0 ? speedIndex : 0);
    cwSpeedWpm = ui->morseSpeed->currentText().toInt();

    m_model = new QSqlTableModel(this);
    m_model->setTable("modes");
    m_model->setEditStrategy(QSqlTableModel::OnFieldChange);
    int callCol = m_model->fieldIndex("callsign");
    if (callCol >= 0) {
        m_model->setSort(callCol, Qt::AscendingOrder);
    }
    m_model->select();

    dxccDb = QSqlDatabase::database("dxcc");
    m_dxccModel = new QSqlTableModel(this, dxccDb);
    m_dxccModel->setTable("modes");
    m_dxccModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    int dxccCallCol = m_dxccModel->fieldIndex("callsign");
    if (dxccCallCol >= 0) {
        m_dxccModel->setSort(dxccCallCol, Qt::AscendingOrder);
    }
    m_dxccModel->select();

    auto setupModesView = [this](QTableView *view, QSqlTableModel *model) {
        if (!view || !model) {
            return;
        }
        view->setModel(model);
        // Single-row selection with light highlight
        view->setSelectionMode(QAbstractItemView::SingleSelection);
        view->setSelectionBehavior(QAbstractItemView::SelectRows);
        view->setStyleSheet(
            "QTableView::item:selected { background: #dfefff; color: palette(text); }");

        view->setColumnHidden(0, true);

        if (!checkboxDelegate) {
            checkboxDelegate = new CheckboxDelegate(this);
        }
        // Custom delegate on the 'bands' column
        for (int i = 2; i < 10; i++)
        {
            view->setItemDelegateForColumn(i, checkboxDelegate);
            view->setColumnWidth(i, 120);
        }
    };

    setupModesView(ui->tableView, m_model);
    setupModesView(ui->dxccTableView, m_dxccModel);

    statusCountsLabel = new QLabel(this);
    statusCountsLabel->setMinimumWidth(260);
    statusCountsLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusInfoLabel = new QLabel(this);
    statusInfoLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->statusbar->addWidget(statusInfoLabel, 1);
    ui->statusbar->addPermanentWidget(statusCountsLabel);
    statusInfoLabel->setText("Ready");
    updateStatusCounts();
    updateModeVisibility();
    ui->statusbar->installEventFilter(this);


    rbnSocket = new QTcpSocket(this);
    connect(rbnSocket, &QTcpSocket::readyRead, this, [this]() {
        const QByteArray data = rbnSocket->readAll();
        if (data.isEmpty()) {
            return;
        }

        rbnBuffer.append(data);
        // qDebug().noquote() << "RBN:" << data;

        static const QRegularExpression rbnLineRegex(
            R"(^DX de\s+\S+:\s+([0-9.]+)\s+([A-Za-z0-9/]+)\b(?:\s+([A-Za-z0-9/]+))?)"
            );
        auto freqToBand = [](double value) -> QString {
            // RBN spots often use kHz (e.g. 14074.0); normalize to MHz.
            double mhz = value;
            if (mhz > 1000.0) {
                mhz /= 1000.0;
            }

            if (mhz >= 3.5 && mhz < 4.0) return "80";
            if (mhz >= 7.0 && mhz < 7.3) return "40";
            if (mhz >= 10.1 && mhz < 10.15) return "30";
            if (mhz >= 14.0 && mhz < 14.35) return "20";
            if (mhz >= 18.068 && mhz < 18.168) return "17";
            if (mhz >= 21.0 && mhz < 21.45) return "15";
            if (mhz >= 24.89 && mhz < 24.99) return "12";
            if (mhz >= 28.0 && mhz < 29.7) return "10";
            return QString();
        };

        if (rbnOutputPaused) {
            if (!rbnLoginSent && rbnBuffer.contains("Please enter your call:")) {
                rbnSocket->write("OG3Z\r\n");
                rbnLoginSent = true;
                qDebug() << "RBN login sent";
            }
            rbnBuffer.clear();
            return;
        }

        while (true) {
            const int newlineIndex = rbnBuffer.indexOf('\n');
            if (newlineIndex < 0) {
                break;
            }

            const QByteArray lineBytes = rbnBuffer.left(newlineIndex);
            rbnBuffer.remove(0, newlineIndex + 1);

            const QString line = QString::fromUtf8(lineBytes).trimmed();
            if (line.isEmpty()) {
                continue;
            }

            //qDebug().noquote() << line;

            const QRegularExpressionMatch match = rbnLineRegex.match(line);
            if (match.hasMatch()) {
                const QString freq = match.captured(1);
                const QString call = match.captured(2);
                const QString callUp = call.trimmed().toUpper();
                const QString mode = match.captured(3).trimmed().toUpper();
                const double freqValue = freq.toDouble();
                const QString band = freqToBand(freqValue);
                // qDebug().noquote() << "RBN spot:" << "call=" << call << "freq=" << freq;

                if (band.isEmpty()) {
                    return;
                }

                QSqlQuery q;
                const QString sql = QString(R"(SELECT "%1" FROM modes WHERE callsign = ? LIMIT 1)").arg(band);
                q.prepare(sql);
                q.addBindValue(callUp);
                if (!q.exec()) {
                    qWarning() << "RBN DB lookup failed:" << q.lastError();
                } else if (q.next()) {
                    const int mask = q.value(0).toInt();
                    //qDebug().noquote() << "RBN in DB:" << callUp;
                    if (!(mask & (1 << 0))) {
                        //qDebug().noquote() << callUp << mode << freq;
                        if (statusInfoLabel && mode == "CW") {
                            statusInfoLabel->setText(QString("%1 %2").arg(callUp, freq));
                        } else {
                            qDebug() << "RBN non-CW:" << callUp << freq << "mode" << (mode.isEmpty() ? "<none>" : mode);
                        }
                    }
                }
            }
        }

        if (!rbnLoginSent && rbnBuffer.contains("Please enter your call:")) {
            rbnSocket->write("OG3Z\r\n");
            rbnLoginSent = true;
            qDebug() << "RBN login sent";
        }
    });
    connect(rbnSocket,
            QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [this](QAbstractSocket::SocketError) {
                qWarning() << "RBN socket error:" << rbnSocket->errorString();
            });
    connect(rbnSocket, &QTcpSocket::connected, this, [this]() {
        qDebug() << "RBN connected";
    });
    connect(rbnSocket, &QTcpSocket::disconnected, this, [this]() {
        qWarning() << "RBN disconnected";
    });
    rbnSocket->connectToHost("telnet.reversebeacon.net", 7000);
    connect(ui->clearButton, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    connect(ui->dxccClearButton, &QPushButton::clicked, this, &MainWindow::onDxccClearClicked);
    connect(m_model, &QAbstractItemModel::dataChanged,
            this, [this](const QModelIndex &, const QModelIndex &, const QVector<int> &) {
                updateStatusCounts();
            });

    connect(ui->actionSettings, &QAction::triggered, this, &MainWindow::showSettingsDialog);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);
    connect(ui->modeCwButton, &QPushButton::clicked, this, [this]() { if (rig) rig->setMode(RIG_MODE_CW); });
    connect(ui->modeUsbButton, &QPushButton::clicked, this, [this]() { if (rig) rig->setMode(RIG_MODE_USB); });
    connect(ui->modeLsbButton, &QPushButton::clicked, this, [this]() { if (rig) rig->setMode(RIG_MODE_LSB); });
    connect(ui->modeFmButton, &QPushButton::clicked, this, [this]() { if (rig) rig->setMode(RIG_MODE_FM); });
    connect(ui->modeAmButton, &QPushButton::clicked, this, [this]() { if (rig) rig->setMode(RIG_MODE_AM); });
    connect(ui->band18Button, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(1800000); });
    connect(ui->band35Button, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(3500000); });
    connect(ui->band7Button, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(7000000); });
    connect(ui->band10Button, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(10000000); });
    connect(ui->band14Button, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(14000000); });
    connect(ui->band18mButton, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(18068000); });
    connect(ui->band21Button, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(21000000); });
    connect(ui->band24Button, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(24900000); });
    connect(ui->band28Button, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(28000000); });
    connect(ui->band50Button, &QPushButton::clicked, this, [this]() { if (rig) rig->setFrequency(50000000); });

    connect(ui->sendOg3zButton, &QPushButton::clicked, this, [this]() { if (rig) rig->sendCw("OG3Z"); });
    connect(ui->send5nnTuOg3zButton, &QPushButton::clicked, this, [this]() { if (rig) rig->sendCw("5NN TU"); });
    connect(ui->sendOg3zGmButton, &QPushButton::clicked, this, [this]() { if (rig) rig->sendCw("GM 5NN TU"); });
    connect(ui->send5nnTuGaButton, &QPushButton::clicked, this, [this]() { if (rig) rig->sendCw("GA 5NN TU"); });
    connect(ui->send5nnTuGeButton, &QPushButton::clicked, this, [this]() { if (rig) rig->sendCw("GE 5NN TU"); });
    connect(ui->send5nnTuOgButton, &QPushButton::clicked, this, [this]() { if (rig) rig->sendCw("OG OG"); });
    connect(ui->sendOgQmButton, &QPushButton::clicked, this, [this]() { if (rig) rig->sendCw("TEST TEST TEST TEST TEST"); });
    connect(ui->logButton, &QPushButton::clicked, this, &MainWindow::onLogClicked);
    connect(ui->morseSpeed, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int) {
                const int wpm = ui->morseSpeed->currentText().toInt();
                if (wpm <= 0) {
                    return;
                }
                cwSpeedWpm = wpm;
                if (rig) {
                    rig->setCwSpeed(wpm);
                }
            });

    connect(ui->leftFrequency, &FrequencyLabel::valueChanged, this, [this](int value, QChar) {
        if (rig) {
            rig->setFrequency(value);
        }
    });
    connect(ui->rightFrequency, &FrequencyLabel::valueChanged, this, [this](int value, QChar) {
        if (!rig) {
            return;
        }
        vfo_t leftVfo = RIG_VFO_CURR;
        if (!rig->readVfo(leftVfo)) {
            return;
        }
        vfo_t rightVfo = RIG_VFO_SUB;
        if (leftVfo == RIG_VFO_A) {
            rightVfo = RIG_VFO_B;
        } else if (leftVfo == RIG_VFO_B) {
            rightVfo = RIG_VFO_A;
        }
        rig->setFrequency(rightVfo, value);
    });

    QSettings settings;
    const int model = settings.value("rig/model", RIG_MODEL_TS590S).toInt();
    const QString port = settings.value("rig/port", "COM7").toString();
    rig = std::make_unique<Rig>(static_cast<rig_model_t>(model), port);

    if (!rig->open()) {
        qDebug() << "Hamlib rig_open failed:" << rig->lastError();
        return;
    }
    rig->setCwSpeed(cwSpeedWpm);

    pollTimer = new QTimer(this);
    pollTimer->setInterval(100);
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
    portCombo.addItem("COM4", "COM4");

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
    vfo_t leftVfo = RIG_VFO_CURR;
    if (rig->readVfo(leftVfo)) {
        ui->leftFrequency->setPrefix(vfoToPrefix(leftVfo));
    }
    int frequency = 0;
    if (rig->readFrequency(leftVfo, frequency)) {
        ui->leftFrequency->setValue(frequency);
    }

    bool splitOn = false;
    vfo_t txVfo = RIG_VFO_CURR;
    if (rig->readSplit(splitOn, txVfo)) {
        ui->rightFrequency->setVisible(splitOn);
        if (splitOn) {
            vfo_t rightVfo = RIG_VFO_SUB;
            if (leftVfo == RIG_VFO_A) {
                rightVfo = RIG_VFO_B;
            } else if (leftVfo == RIG_VFO_B) {
                rightVfo = RIG_VFO_A;
            }
            int subFrequency = 0;
            if (rig->readFrequency(rightVfo, subFrequency)) {
                ui->rightFrequency->setValue(subFrequency);
                ui->rightFrequency->setPrefix(vfoToPrefix(rightVfo));
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

void MainWindow::updateStatusCounts()
{
    int cw = 0, ph = 0, ft8 = 0, ft4 = 0;

    static const QStringList bands = {
        "10","12","15","17","20","30","40","80"
    };

    QSqlQuery q;

    for (const QString &band : bands) {
        const QString sql = QString(R"(SELECT "%1" FROM modes)").arg(band);
        if (!q.exec(sql)) {
            qWarning() << "Count query failed:" << q.lastError();
            return;
        }

        while (q.next()) {
            const int mask = q.value(0).toInt();
            if (mask & (1 << 0)) cw++;
            if (mask & (1 << 1)) ph++;
            if (mask & (1 << 2)) ft8++;
            if (mask & (1 << 3)) ft4++;
        }
    }

    const int total = cw * 10 + ph * 5 + ft8 * 2 + ft4 * 2;

    statusCountsLabel->setText(
        QString("CW:%1  PH:%2  FT8:%3  FT4:%4  TOTAL:%5")
            .arg(cw)
            .arg(ph)
            .arg(ft8)
            .arg(ft4)
            .arg(total)
        );
}

void MainWindow::updateModeVisibility()
{
    // modeVisible[0] = ui->cwCheckBox->isChecked();
    // modeVisible[1] = ui->phCheckBox->isChecked();
    // modeVisible[2] = ui->ft8CheckBox->isChecked();
    // modeVisible[3] = ui->ft4CheckBox->isChecked();

    // if (checkboxDelegate) {
    //     checkboxDelegate->setModeVisibility(modeVisible);
    // }
    // ui->tableView->viewport()->update();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->statusbar && event->type() == QEvent::MouseButtonPress) {
        rbnOutputPaused = !rbnOutputPaused;
        if (statusInfoLabel) {
            statusInfoLabel->setStyleSheet(rbnOutputPaused ? "color: red;" : "");
        }
        const QString text = statusInfoLabel ? statusInfoLabel->text().trimmed() : QString();
        static const QRegularExpression re(R"(^(\S+)\s+([0-9.]+))");
        const QRegularExpressionMatch match = re.match(text);
        if (match.hasMatch()) {
            const QString call = match.captured(1);
            const QString freq = match.captured(2);
            if (ui->callLabel) {
                ui->callLabel->setText(call);
            }
            if (ui->freqLabel) {
                ui->freqLabel->setText(freq);
            }
            if (rig) {
                bool ok = false;
                const double freqValue = freq.toDouble(&ok);
                if (ok) {
                    int hz = 0;
                    if (freqValue >= 1000.0) {
                        hz = static_cast<int>(freqValue * 1000.0);
                    } else {
                        hz = static_cast<int>(freqValue * 1000000.0);
                    }
                    rig->setFrequency(hz);
                    if (ui->leftFrequency) {
                        ui->leftFrequency->setValue(hz);
                    }
                }
            }
        }
        return true;
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::onClearClicked()
{
    const auto response = QMessageBox::question(
        this,
        "Confirm Clear",
        "Set every band value to 0 for all callsigns?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (response != QMessageBox::Yes) {
        return;
    }

    QSqlQuery q;
    const QString sql = R"(
        UPDATE modes SET
            "10" = 0,
            "12" = 0,
            "15" = 0,
            "17" = 0,
            "20" = 0,
            "30" = 0,
            "40" = 0,
            "80" = 0
    )";

    if (!q.exec(sql)) {
        qWarning() << "Clear failed:" << q.lastError();
        if (statusInfoLabel) {
            statusInfoLabel->setText("Clear failed");
        }
        return;
    }

    if (m_model) {
        m_model->select();
    }
    updateStatusCounts();
    if (statusInfoLabel) {
        statusInfoLabel->setText("Cleared all data");
    }
}

void MainWindow::onDxccClearClicked()
{
    const auto response = QMessageBox::question(
        this,
        "Confirm Clear",
        "Set every band value to 0 for all callsigns?",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No
        );

    if (response != QMessageBox::Yes) {
        return;
    }

    QSqlQuery q(dxccDb);
    const QString sql = R"(
        UPDATE modes SET
            "10" = 0,
            "12" = 0,
            "15" = 0,
            "17" = 0,
            "20" = 0,
            "30" = 0,
            "40" = 0,
            "80" = 0
    )";

    if (!q.exec(sql)) {
        qWarning() << "DXCC clear failed:" << q.lastError();
        if (statusInfoLabel) {
            statusInfoLabel->setText("DXCC clear failed");
        }
        return;
    }

    if (m_dxccModel) {
        m_dxccModel->select();
    }
    if (statusInfoLabel) {
        statusInfoLabel->setText("Cleared DXCC data");
    }
}

void MainWindow::onLogClicked()
{
    const QString call = ui->callLabel ? ui->callLabel->text().trimmed().toUpper() : QString();
    const QString freqText = ui->freqLabel ? ui->freqLabel->text().trimmed() : QString();
    if (call.isEmpty() || freqText.isEmpty()) {
        if (statusInfoLabel) {
            statusInfoLabel->setText("Log failed");
        }
        return;
    }

    const QString band = bandFromFrequencyText(freqText);
    if (band.isEmpty()) {
        if (statusInfoLabel) {
            statusInfoLabel->setText("Log failed");
        }
        return;
    }

    QSqlQuery q;
    const QString sql = QString(R"(UPDATE modes SET "%1" = (COALESCE("%1", 0) | 1) WHERE callsign = ?)")
                            .arg(band);
    q.prepare(sql);
    q.addBindValue(call);
    if (!q.exec()) {
        qWarning() << "Log update failed:" << q.lastError();
        if (statusInfoLabel) {
            statusInfoLabel->setText("Log failed");
        }
        return;
    }

    if (m_model) {
        m_model->select();
    }
    updateStatusCounts();
    if (statusInfoLabel) {
        statusInfoLabel->setText("Logged");
    }
    rbnOutputPaused = false;
    if (statusInfoLabel) {
        statusInfoLabel->setStyleSheet("");
    }
}
