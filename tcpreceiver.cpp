#include "tcpreceiver.h"

#include <QDebug>

TcpReceiver::TcpReceiver(const QString &host, quint16 port, QObject *parent)
    : QObject(parent)
    , m_host(host)
    , m_port(port)
{
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &TcpReceiver::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, [this]() {
        qDebug() << "TCP disconnected:" << m_host << m_port;
    });
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpReceiver::onReadyRead);
    connect(m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, [this](QAbstractSocket::SocketError) {
                qWarning() << "TCP error:" << m_socket->errorString();
            });
}

void TcpReceiver::start()
{
    if (!m_socket) {
        return;
    }
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        m_socket->connectToHost(m_host, m_port);
    }
}

void TcpReceiver::stop()
{
    if (!m_socket) {
        return;
    }
    m_socket->disconnectFromHost();
}

void TcpReceiver::connected()
{
    qDebug() << "connected...";
    if (m_socket) {
        m_socket->write("og3z\r\n");
    }
}

void TcpReceiver::onReadyRead()
{
    if (!m_socket) {
        return;
    }
    const QByteArray data = m_socket->readAll();
    if (!data.isEmpty()) {
        qDebug().noquote() << "TCP data:" << data;
    }
}
