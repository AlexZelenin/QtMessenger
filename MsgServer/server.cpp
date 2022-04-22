#include "server.h"

#include "sthread.h"

#include <QThread>

Server::Server(QObject *parent)
    : QTcpServer(parent)
{

}

bool Server::start(quint16 port, const QString &serverName)
{
    m_serverName = serverName;
    return listen(QHostAddress::Any, port);
}

void Server::stop()
{
    emit stopSession();
    close();
}

void Server::incomingConnection(qintptr handle)
{
    SessionThread *sth = new SessionThread(handle, m_serverName);
    QThread *thread = new QThread;

    sth->moveToThread(thread);

    connect(sth, &SessionThread::newClientConnected, this, &Server::newClientConnected, Qt::DirectConnection);
    connect(sth, &SessionThread::clientDisconnected, this, &Server::clientDisconnected, Qt::DirectConnection);

    connect(thread, &QThread::started, sth, &SessionThread::runThread, Qt::DirectConnection);
    connect(this, &Server::stopSession, sth, &SessionThread::stop);
    connect(this, &Server::stopSession, thread, &QThread::quit);

    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    connect(thread, &QThread::finished, sth, &SessionThread::deleteLater);

    thread->start();
}
