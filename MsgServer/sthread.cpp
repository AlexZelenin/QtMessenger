#include "sthread.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDataStream>

#include "serialize.pb.h"

SessionThread::SessionThread(int socketDescriptor, const QString& serverName, QObject* parent)
    : QObject(parent)
    , m_socketDescriptor(socketDescriptor)
    , m_serverName(serverName)
    , m_nextBlockSize(0)
{

    /*
     * Соединяем сигнал send со слотом отправки сообщений sendMessage
    */
    connect(this, &SessionThread::send, this, &SessionThread::sendMessage, Qt::DirectConnection);
}

SessionThread::~SessionThread()
{

}


/*
 * Метод запуска сессии
*/
void SessionThread::runThread()
{

    /*
     * Создаем tcp сокети устанавливаем ему опцию KeepAlive для реагирования на разрыв соедниения
     * ?? Не совсем с этим разобрался. Везде пишут что реагирует только через 2 часа
    */
    m_socket = new QTcpSocket;
    m_socket->setSocketOption(QAbstractSocket::KeepAliveOption, true);

    /*
     * Устанавливаем дескриптор сокета.
    */
    if (!m_socket->setSocketDescriptor(m_socketDescriptor)) {
        emit error(m_socket->error());
        return;
    }

    /*
     * Обработка на чтение входящих сообщений, разъединение, ошибок и состояния соедниения
    */
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(readyRead()), Qt::DirectConnection);
    connect(m_socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(m_socket, &QTcpSocket::errorOccurred, this, &SessionThread::errorConnection, Qt::DirectConnection);
    connect(m_socket, &QTcpSocket::stateChanged, this, &SessionThread::connectionState, Qt::DirectConnection);

}


/*
 * Метод чтения входящих сообщений
*/
void SessionThread::readyRead()
{
    QString data;
    data.clear();

    /*
     * Создаем объект потока данных в который будут записываться входящие данные
    */
    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_5_15);

    /*
     * Заходим в цикл чтения всех пакетов
     * Сначала необходимо получить общий размер всех данных, поэтому проверяем на то что первый пакет
     * не должен быть меньше 2х байт (в противном случае выходим из цикла). Далее записываем размер в
     * приватную переменную класса типа quint16.
     * Далее проверяем что бы размер входящего пакета не был меньше чем размер указаный в переменной nextBlockSize
     * После чего записываем данные в переменную data типа QString
     * и обнуляем nextBlockSize
    */
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

    /*
     * Переводим данные в тип Json
    */
    QJsonParseError jsonError;
    const QJsonDocument clientDoc = QJsonDocument::fromJson(data.toUtf8(), &jsonError);

    /*
     * Проверка на валидность структуры Json
    */
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "Error incoming data:" << jsonError.errorString();
        return;
    }

    const QJsonObject clientObj = clientDoc.object();

    /*
     * Проверяем, если есть ключ 'clientname', то передаем сигнал с именем поключенного клиента
     * и его поток. Далее формируем Json с именем сервера и отправляем в ответ клиенту. Говоря тем самым о успешном поключении клиента
    */
    if (clientObj.contains("clientname")) {
        m_clientName = clientObj.value("clientname").toString();
        emit newClientConnected(m_clientName, this);

        QJsonObject obj;
        obj.insert("servername", QJsonValue::fromVariant(m_serverName));

        QJsonDocument doc(obj);
        emit send(doc.toJson());
        return;
    }

    /*
     * Сигналом сообщаем о полученных данных от клиента
    */
    emit recive(data);
}

/*
 * Уведомляем о том что клиент октлючился и закрываем сокет
*/
void SessionThread::disconnected()
{
    emit clientDisconnected(m_clientName);
    m_socket->close();
}

/*
 * Остановка поключения
*/
void SessionThread::stop()
{
    /*
     * Проверяем если клиент уже отключился то только помечаем объект сокета на удаление
     * в противном случае закрываем сокет ...
    */
    if (!m_socket->isOpen()) {
        m_socket->deleteLater();
    } else {
        m_socket->close();
        m_socket->deleteLater();
    }
}

/*
 *
*/
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


/*
 * Метод отправки сообщений клиенту
*/
void SessionThread::sendMessage(const QString &msg)
{
    m_data.clear();
    QDataStream out(&m_data, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_15);

    /*
     * Перед отправкой сообщения в переменную потока данных сначала резирвируем первые 2 байта
     * для последующей записи в них размера передаваемых данных, далее записываем сами данные
     * после чего методом seek переходим в начало и записываем уже сам размер данных
    */
    out << quint16(0) << msg;
    out.device()->seek(0);
    out << quint16(m_data.size() - sizeof(quint16));

    /*
     * Отправляем данные клиенту
    */
    m_socket->write(m_data);
}
