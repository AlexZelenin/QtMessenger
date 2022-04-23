#include "client.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QThread>
#include <QDataStream>


#if defined(Q_OS_LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#elif defined(Q_OS_WINDOWS)
#endif

#include <QDebug>

Client::Client(QObject *parent)
    : QObject{parent}
    , m_nextBlockSize(0)
{

}

bool Client::clientConnect(const QString& hostName, const int& port, const QString& clientName)
{
    m_clientName = clientName;
    m_hostName = hostName;
    m_port = port;
    m_socket = new QTcpSocket(this);
    m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);

    connect(m_socket, &QTcpSocket::connected, this, &Client::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::clientDisconnect);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::readyRead);

    connect(m_socket, &QTcpSocket::stateChanged, this, &Client::connectionState);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &Client::errorConnection);

    m_socket->connectToHost(hostName, port);

    if(!m_socket->waitForConnected(3000)) {
        return false;
    }

    return true;
}

void Client::clientDisconnect()
{
    m_socket->close();
}

void Client::connected()
{
    QJsonObject jobj;
    jobj.insert("clientname", QJsonValue::fromVariant(m_clientName));
    QJsonDocument doc(jobj);

    auto data = doc.toJson();
    qDebug() << data;

    sendMessage(data);
}

void Client::readyRead()
{
    QString data;
    data.clear();

    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_5_15);

    while(true) {
        if (!m_nextBlockSize) {
            if (m_socket->bytesAvailable() < sizeof(quint16)) {
                break;
            }
            in >> m_nextBlockSize;
        }

        if (m_socket->bytesAvailable() < m_nextBlockSize) {
            break;
        }

        in >> data;
        m_nextBlockSize = 0;
    }

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &jsonError);

    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "Data error:" << jsonError.errorString();
    }

    QJsonObject obj = doc.object();
    if (obj.contains("servername")) {
        const QString serverName = obj.value("servername").toString();
        emit toServerConnected(serverName);
        return;
    }

    emit reciveMessage(data);

}

void Client::sendMessage(const QString &msg)
{
    m_data.clear();
    QDataStream out(&m_data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << msg;
    out.device()->seek(0);
    out << quint16(m_data.size() - sizeof(quint16));
    m_socket->write(m_data);
}

void Client::errorConnection(QAbstractSocket::SocketError error)
{
    qDebug() << "Error Connection" << error;

    switch(error) {
    case QAbstractSocket::ConnectionRefusedError:
        emit connectionRefuse();
    default:
        break;
    }
}

void Client::connectionState(QAbstractSocket::SocketState state)
{
    qDebug() << "Socket State:" << state;

    switch(state) {
    case QAbstractSocket::ConnectedState:
        emit connectedState();
        break;
    default:
        break;
    }
}
