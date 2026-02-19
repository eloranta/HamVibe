#include "tcpreceiver.h"

#include <QDebug>
#include <QRegularExpression>
#include <QSqlQuery>

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

std::tuple<const QString, const QString, const QString, const QString, const QString> parseLine(const QString &line) {
    const int colon = line.indexOf(':');
    //if (colon == -1) return; // TODO:

    const QString sender = line.left(colon).mid(6);
    const QString freq = line.mid(colon+1, 24-colon).trimmed();
    const QString call = line.mid(26, 13).trimmed();
    const QString msg = line.mid(39, 30).trimmed();
    const QString time = line.mid(70, 4).trimmed();

    return {sender, freq, call, msg, time };
}

void TcpReceiver::onReadyRead()
{
    const QString line = QString::fromUtf8(m_socket->readAll());
    if (line.isEmpty()) return;
    if (!line.startsWith("DX de")) return;

    auto [sender, freq, call, msg, time] = parseLine(line);

    //qDebug() << line;
    //qDebug() << sender << freq << call << msg << time;

    if (!sender.isEmpty() && !freq.isEmpty() && !call.isEmpty() && !time.isEmpty()) {
        QString band;
        bool ok = false;
        const double freqVal = freq.trimmed().toDouble(&ok);
        if (ok) {
            double mhz = freqVal;
            if (mhz > 1000.0) {
                mhz /= 1000.0;
            }
            if (mhz >= 1.8 && mhz < 2.0) band = "160";
            else if (mhz >= 3.5 && mhz < 4.0) band = "80";
            else if (mhz >= 7.0 && mhz < 7.3) band = "40";
            else if (mhz >= 10.1 && mhz < 10.15) band = "30";
            else if (mhz >= 14.0 && mhz < 14.35) band = "20";
            else if (mhz >= 18.068 && mhz < 18.168) band = "17";
            else if (mhz >= 21.0 && mhz < 21.45) band = "15";
            else if (mhz >= 24.89 && mhz < 24.99) band = "12";
            else if (mhz >= 28.0 && mhz < 29.7) band = "10";
            else if (mhz >= 50.0 && mhz < 54.0) band = "6";
            else if (mhz >= 144.0 && mhz < 148.0) band = "2";
        }
        QString mode = "??";
        const QString msgUp = msg.toUpper();
        if (msgUp.contains("SAT")) mode = "SAT";
        else if (msgUp.contains("CW")) mode = "CW";
        else if (msgUp.contains("RTTY") || msgUp.contains("FT8") || msgUp.contains("FT4") || msgUp.contains("DATA")) mode = "RT";
        else if (msgUp.contains("SSB") || msgUp.contains("PHONE")) mode = "Ph";
        if (mode == "??" && ok) {
            static const QSet<QString> kRtFreqs = {
                "1840.0","1837.0","3573.0","3575.0","7074.0","7047.5",
                "10136.0","10140.0","14074.0","14080.0","18100.0","18104.0",
                "21074.0","21140.0","24915.0","24919.0","28074.0","28180.0",
                "50313.0","50318.0"
            };
            const QString freqKey = QString::number(freqVal, 'f', 1);
            if (kRtFreqs.contains(freqKey)) {
                mode = "RT";
            } else if (!band.isEmpty()) {
                double mhz = freqVal;
                if (mhz > 1000.0) {
                    mhz /= 1000.0;
                }
                double bandStart = 0.0;
                if (band == "160") bandStart = 1.8;
                else if (band == "80") bandStart = 3.5;
                else if (band == "40") bandStart = 7.0;
                else if (band == "30") bandStart = 10.1;
                else if (band == "20") bandStart = 14.0;
                else if (band == "17") bandStart = 18.068;
                else if (band == "15") bandStart = 21.0;
                else if (band == "12") bandStart = 24.89;
                else if (band == "10") bandStart = 28.0;
                else if (band == "6") bandStart = 50.0;
                else if (band == "2") bandStart = 144.0;

                if (bandStart > 0.0) {
                    if (mhz >= bandStart && mhz < bandStart + 0.1) {
                        mode = "CW";
                    } else {
                        mode = "Ph";
                    }
                }
            }
        }

        const QString country = m_country.GetCountry(call).toUpper();
        if (!country.isEmpty() && !band.isEmpty()) {
            QSqlQuery q;
            const QString sql = QString("SELECT COALESCE(\"%1\", '') FROM dxcc WHERE entity = ? LIMIT 1").arg(band);
            q.prepare(sql);
            q.addBindValue(country);
            if (q.exec() && q.next()) {
                const QString value = q.value(0).toString();
                if (value.isEmpty()) {
                    qDebug().noquote() << time << call << freq << band << mode << country;
                    emit spotReceived(time.trimmed(), call.trimmed(), freq.trimmed(), mode.trimmed(), country.trimmed(), sender.trimmed());
                }
            } else {
                qDebug().noquote() << "UNKNOWN COUNTRY" << call << country;
            }
        }
    } else if (line.startsWith("DX de")) {
        // qDebug().noquote() << line;
    }
}
