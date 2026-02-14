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
public slots:
    //void onQsoLogged(const QString &call, const QString &band, const QString &mode);
    //void onAddClicked();
    void onClearClicked();
    void onLogClicked();
    void onDxccReadAdiClicked();

private:
    Ui::MainWindow *ui;
    class QSqlTableModel *m_model = nullptr;
    class QSqlTableModel *m_dxccModel = nullptr;
    class WwaDelegate *checkboxDelegate = nullptr;

    class QTcpSocket *rbnSocket = nullptr;
    QByteArray rbnBuffer;
    bool rbnLoginSent = false;
    bool rbnOutputPaused = false;
    class QLabel *statusInfoLabel = nullptr;
    class QLabel *statusCountsLabel = nullptr;
    void updateStatusCounts();
    void updateModeVisibility();

    std::unique_ptr<Rig> rig;
    QTimer *pollTimer = nullptr;
    int cwSpeedWpm = 30;
    bool lsbSelected = true;
    bool fmSelected = true;
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void togglePtt();
    void toggleLsbUsb();
    void setCwMode();
    void toggleFmAm();
    void poll();
    void showSettingsDialog();
    void showAboutDialog();

private:
    double interpolateSmeterDb(int value) const;
};
#endif // MAINWINDOW_H
