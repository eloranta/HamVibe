#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Rig.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

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

private:
    Ui::MainWindow *ui;
    Rig rig;
    bool split = false;
    vfo_t rxVfo = RIG_VFO_NONE;
    vfo_t txVfo = RIG_VFO_NONE;
};
#endif // MAINWINDOW_H
