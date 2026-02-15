#include "tcpreceiver.h"

#include <QDebug>
#include <QRegularExpression>

TcpReceiver::TcpReceiver(const QString &host, quint16 port, QObject *parent)
    : QObject(parent)
    , m_host(host)
    , m_port(port)
{
    m_country.init();
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
    if (data.isEmpty()) {
        return;
    }
    m_buffer.append(QString::fromUtf8(data));
    int newline = -1;
    static const QRegularExpression re(
        R"(^DX de\s+(\S+):\s*([0-9.]+)\s+(\S+)\s+(.*?)\s+(\d{3,4}Z)\b.*$)");
    while ((newline = m_buffer.indexOf('\n')) >= 0) {
        QString line = m_buffer.left(newline);
        m_buffer.remove(0, newline + 1);
        line = line.trimmed();
        if (line.isEmpty()) {
            continue;
        }
        QString sender;
        QString freq;
        QString call;
        QString msg;
        QString time;

        if (line.startsWith("DX de")) {
            const int colon = line.indexOf(':');
            if (colon > 0) {
                sender = line.mid(6, colon - 6).trimmed();
                const QString rest = line.mid(colon + 1);
                if (rest.size() >= 10) {
                    freq = rest.mid(0, 10).trimmed();
                }
                if (rest.size() >= 22) {
                    call = rest.mid(10, 12).trimmed();
                }
                if (rest.size() >= 52) {
                    msg = rest.mid(22, 30).trimmed();
                } else if (rest.size() > 22) {
                    msg = rest.mid(22).trimmed();
                }
                if (rest.size() >= 57) {
                    time = rest.mid(52).trimmed();
                    const int space = time.indexOf(' ');
                    if (space > 0) {
                        time = time.left(space);
                    }
                }
            }
        }

        if (sender.isEmpty() || freq.isEmpty() || call.isEmpty() || time.isEmpty()) {
            const QRegularExpressionMatch match = re.match(line);
            if (match.hasMatch()) {
                sender = match.captured(1);
                freq = match.captured(2);
                call = match.captured(3);
                msg = match.captured(4).trimmed();
                time = match.captured(5);
            }
        }

        if (!sender.isEmpty() && !freq.isEmpty() && !call.isEmpty() && !time.isEmpty()) {
            freq.replace(QRegularExpression(R"(\s*\.\s*)"), ".");
            QString t = time.trimmed();
            if (t.endsWith('Z')) {
                t.chop(1);
            }
            t.remove(QRegularExpression(R"([^\d])"));
            const QString country = m_country.GetCountry(call);
            qDebug().noquote() << sender << freq << call << msg << t << country;
        } else if (line.startsWith("DX de")) {
            qDebug().noquote() << line;
        }
    }
}
