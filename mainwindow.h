#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QTcpSocket>
#include <QSqlTableModel>
#include <QFile>
#include <QTextStream>
#include <QSet>
#include <QHash>

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
    QHash<QString, QString> prefixToCountry;

    void setupSpotsSocket();
    void parseCtyFile();
    QString findCountryForCall(const QString &call) const;
    bool isLogSlotEmpty(const QString &country, int meters) const;

private slots:
    void onSpotsConnected();
    void onSpotsReadyRead();
};
#endif // MAINWINDOW_H
