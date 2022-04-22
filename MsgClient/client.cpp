#include "client.h"

#include <QJsonDocument>
#include <QJsonObject>

#include <QDebug>

Client::Client(QObject *parent)
    : QObject{parent}
{

}

bool Client::clientConnect(const QString& hostName, const int& port, const QString& clientName)
{
    m_clientName = clientName;
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QTcpSocket::connected, this, &Client::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::clientDisconnect);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::readyRead);

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

    m_socket->write(data);
}

void Client::readyRead()
{
    QByteArray data = m_socket->readAll();

    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &jsonError);

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
    m_socket->write(msg.toUtf8());
}
