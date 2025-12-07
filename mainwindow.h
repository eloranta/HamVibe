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
#include <QSortFilterProxyModel>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class SpotsFilterProxy;

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
    SpotsFilterProxy *spotsProxy = nullptr;
    QTcpSocket *spotSocket = nullptr;
    QHash<QString, QString> prefixToCountry;
    QHash<QString, QString> countryToContinent;

    void setupSpotsSocket();
    void parseCtyFile();
    QString findCountryForCall(const QString &call) const;
    QString continentForCountry(const QString &country) const;
    bool isLogSlotEmpty(const QString &country, int meters) const;
    void updateBandFilter();

private slots:
    void onSpotsConnected();
    void onSpotsReadyRead();
};
#endif // MAINWINDOW_H
