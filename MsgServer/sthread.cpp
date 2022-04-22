#include "sthread.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>



SessionThread::SessionThread(int socketDescriptor, const QString& serverName, QObject* parent)
    : QObject(parent)
    , m_socketDescriptor(socketDescriptor)
    , m_serverName(serverName)
{
}

void SessionThread::runThread()
{
    m_socket = new QTcpSocket;

    m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);

    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        emit error(m_socket->error());
        return;
    }

    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_socket, &QTcpSocket::errorOccurred, this, &SessionThread::errorConnection);
}

void SessionThread::readyRead()
{
    QByteArray data = m_socket->readAll();

    QJsonParseError jsonError;
    const QJsonDocument clientDoc = QJsonDocument::fromJson(data, &jsonError);

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
        m_socket->write(doc.toJson());
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

void SessionThread::errorConnection(const QAbstractSocket::SocketError & error)
{
    switch(error) {
    case QAbstractSocket::ConnectionRefusedError:
        qDebug() << "Connection refuse";
        break;
    }
}

void SessionThread::sendMessage(const QString &msg)
{
    m_socket->write(msg.toUtf8());
}
