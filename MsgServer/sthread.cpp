#include "sthread.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDataStream>



SessionThread::SessionThread(int socketDescriptor, const QString& serverName, QObject* parent)
    : QObject(parent)
    , m_socketDescriptor(socketDescriptor)
    , m_serverName(serverName)
    , m_nextBlockSize(0)
{
}

void SessionThread::runThread()
{
    m_socket = new QTcpSocket;
    m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);


#if defined(Q_OS_WINDOWS)
#elif defined(Q_OS_LINUX)
#endif

    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        emit error(m_socket->error());
        return;
    }

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_socket, &QTcpSocket::errorOccurred, this, &SessionThread::errorConnection);
    connect(m_socket, &QTcpSocket::stateChanged, this, &SessionThread::connectionState);
}

void SessionThread::readyRead()
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
    const QJsonDocument clientDoc = QJsonDocument::fromJson(data.toUtf8(), &jsonError);

    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "Error incoming data:" << jsonError.errorString();
        return;
    }

    const QJsonObject clientObj = clientDoc.object();

    if (clientObj.contains("clientname")) {
        m_clientName = clientObj.value("clientname").toString();
        emit newClientConnected(m_clientName, this);

        QJsonObject obj;
        obj.insert("servername", QJsonValue::fromVariant(m_serverName));

        QJsonDocument doc(obj);
        sendMessage(doc.toJson());
        return;
    }

    emit recivedMessage(data);
}

void SessionThread::disconnected()
{
    emit clientDisconnected(m_clientName);
    m_socket->deleteLater();
}

void SessionThread::stop()
{
    m_socket->close();
}

void SessionThread::errorConnection(QAbstractSocket::SocketError error)
{
    qDebug() << error;
    switch(error) {
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << "Connection refuse";
        break;
    }
}

void SessionThread::connectionState(QAbstractSocket::SocketState state)
{
    switch(state) {
    case QAbstractSocket::ConnectedState:
        qDebug() << "Connected State";
    case QAbstractSocket::UnconnectedState:
        qDebug() << "Unconnected State";
    case QAbstractSocket::ClosingState:
        qDebug() << "Closing State";
    }
}

void SessionThread::sendMessage(const QString &msg)
{
    m_data.clear();
    QDataStream out(&m_data, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << msg;
    out.device()->seek(0);
    out << quint16(m_data.size() - sizeof(quint16));
    m_socket->write(m_data);
}
