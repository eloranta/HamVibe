#ifndef TCPLISTENER_H
#define TCPLISTENER_H

#include <QObject>
#include <QTcpSocket>

class TcpListener : public QObject
{
    Q_OBJECT
public:
    explicit TcpListener(const QString &host, quint16 port, QObject *parent = nullptr);

    void start();
    void stop();

private slots:
    void connected();
    void onReadyRead();

private:
    QString m_host;
    quint16 m_port = 0;
    QTcpSocket *m_socket = nullptr;
};

#endif // TCPLISTENER_H
