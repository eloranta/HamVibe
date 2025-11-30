#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QTcpSocket>
#include <QSqlTableModel>
#include <QFile>
#include <QTextStream>

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
    QSqlDatabase db;
    QSqlTableModel *spotsModel = nullptr;
    QTcpSocket *spotSocket = nullptr;

    void setupSpotsSocket();
    void parseCtyFile();

private slots:
    void onSpotsConnected();
    void onSpotsReadyRead();
};
#endif // MAINWINDOW_H
