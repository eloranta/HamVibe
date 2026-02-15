#ifndef TCPRECEIVER_H
#define TCPRECEIVER_H

#include <QObject>
#include <QTcpSocket>
#include "country.h"

class TcpReceiver : public QObject
{
    Q_OBJECT
public:
    explicit TcpReceiver(const QString &host, quint16 port, QObject *parent = nullptr);

    void start();
    void stop();

private slots:
    void connected();
    void onReadyRead();

private:
    QString m_host;
    quint16 m_port = 0;
    QTcpSocket *m_socket = nullptr;
    QString m_buffer;
    Country m_country;
};

#endif // TCPRECEIVER_H
