#include "server.h"


Server::Server(QObject *parent)
    : QTcpServer(parent)
{

}

bool Server::start(quint16 port, const QString &serverName)
{
    m_serverName = serverName;

    /*
     * Запускаем сервер на всех Ip адресах машины и с указаным пользователем портом
    */
    return listen(QHostAddress::Any, port);
}

void Server::stop()
{
    /*
     * сигналом stopSession уведомляем о закрытии сессии и ее потока
    */
    emit stopSession();
    /*
     * Закрываем сервер
    */
    close();
}


/*
 * Новое соединение
*/
void Server::incomingConnection(qintptr handle)
{
    /*
     * Для нового соединения создаем сессию и ее поток
    */
    SessionThread *sth = new SessionThread(handle, m_serverName);
    QThread *thread = new QThread;

    /*
     * Перемещаем сессию в новый поток
    */
    sth->moveToThread(thread);

    /*
     * Перекидываем события из сессии в класс основного окна через класс Server
     * SIGNAL -> SIGNAL
    */
    connect(sth, &SessionThread::newClientConnected, this, &Server::newClientConnected, Qt::QueuedConnection);
    connect(sth, &SessionThread::clientDisconnected, this, &Server::clientDisconnected, Qt::QueuedConnection);

    /*
     * Подписываем на события запуска потока запуск сессии.
    */
    connect(thread, &QThread::started, sth, &SessionThread::runThread, Qt::DirectConnection);

    /*
     * Обработка события заверщения сессии. Остановка сессии и выход из потока
    */
    connect(this, &Server::stopSession, sth, &SessionThread::stop, Qt::QueuedConnection);
    connect(this, &Server::stopSession, thread, &QThread::quit, Qt::QueuedConnection);

    /*
     * По завершению потока помечаем на удаление объектов QThread и SessionThread
    */
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, sth, &SessionThread::deleteLater);

    thread->start();
}
