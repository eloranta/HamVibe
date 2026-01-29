#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <memory>
#include "rig.h"

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

private:
    Ui::MainWindow *ui;

    std::unique_ptr<Rig> rig;
    QTimer *pollTimer = nullptr;
    bool lsbSelected = true;
    bool fmSelected = true;

private slots:
    void togglePtt();
    void toggleLsbUsb();
    void setCwMode();
    void toggleFmAm();
    void poll();
    void showSettingsDialog();
    void showAboutDialog();
};
#endif // MAINWINDOW_H
