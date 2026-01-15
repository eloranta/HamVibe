#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Rig.h"
#include <QMainWindow>
#include <QVector>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onLeftFrequencyChanged(int value, QChar prefix);
    void onRightFrequencyChanged(int value, QChar prefix);
    void onSplitToggled(bool enabled);
    void onSwapButtonClicked();
    void onCopyButtonClicked();
    void onBandButtonClicked();

private:
    struct BandStep {
        int freq;
        rmode_t mode;
    };
    struct BandConfig {
        const char *key;
        QPushButton *button;
        BandStep steps[4];
    };

    void initBandConfigs();
    void loadBandSettings();
    void saveBandFrequency(int bandIndex, int level, int frequency);
    void updateModeLabel(rmode_t mode);
    void setSelectedBandButton(QPushButton *button);

    Ui::MainWindow *ui;
    Rig rig;
    bool split = false;
    vfo_t rxVfo = RIG_VFO_NONE;
    vfo_t txVfo = RIG_VFO_NONE;
    QVector<BandConfig> bandConfigs;
    int bandLevels[10] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    int bandSavedFreqs[10][4] = {};
    int currentBandIndex = -1;
    QPushButton *selectedBandButton = nullptr;
};
#endif // MAINWINDOW_H
