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


/*
 * Подключение к серверу
 * Создается новый сокет
 * Далее идет поключение сигналов слотов на обработку событий
 * (подключен, отключеие, чтение, состояние и ошибки)
*/
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

    /*
     * Подключемся к серверу
    */
    m_socket->connectToHost(hostName, port);

    /*
     * Если в течении 3х секунд не подключились
     * то возвращаем false
    */
    if(!m_socket->waitForConnected(3000)) {
        return false;
    }

    return true;
}

void Client::clientDisconnect()
{
    // Закрываем сокет
    m_socket->close();
}

void Client::connected()
{
    /*
     * После успешного соединения с сервером создлаем Json с ключом clientname
     * в котором передаем имя клиента
     * и отправляем на сервер
    */
    QJsonObject jobj;
    jobj.insert("clientname", QJsonValue::fromVariant(m_clientName));
    QJsonDocument doc(jobj);

    auto data = doc.toJson();
    qDebug() << data;

    sendMessage(data);
}

/*
 * Чтение данных с сервера по аналогии как на сервере
 * Создается переменная потока данных, в которую записываются входящие пакеты
 * далее проверятся на наличие ключа 'servername' если есть то сигналом toServerConnected сообщаем о успешном поключении
 * если нету ключа servername то сигналом уведомляем о входящих данных
*/
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

/*
 * Отправка данных.
 * Сначала идет размер данных потом сами данные
*/
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

/*
 * Обработчик ошибок сети
*/
void Client::errorConnection(QAbstractSocket::SocketError error)
{
    qDebug() << "Error Connection" << error;

    switch(error) {
    case QAbstractSocket::ConnectionRefusedError:
        emit connectionRefuse();
        break;
    case QAbstractSocket::NetworkError:
        emit networkError(tr("Ошибка сети."));
        break;
    case QAbstractSocket::HostNotFoundError:
        emit hostNotFound(tr("Сервер по указанному сетевому адресу и порту не обнаружен."));
        break;
    case QAbstractSocket::SocketTimeoutError:
        emit socketTimeOut(tr("Время ожидания подключения к серверу истекло."));
        break;
    default:
        break;
    }
}

/*
 * Обработчик состояния подлючения
*/
void Client::connectionState(QAbstractSocket::SocketState state)
{
    qDebug() << "Socket State:" << state;

    switch(state) {
    case QAbstractSocket::ConnectedState:
        emit connectedState();
        break;
    case QAbstractSocket::ClosingState:
        emit closingState();
        break;
    default:
        break;
    }
}
