#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QThread>

#include "sthread.h"

class Server : public QTcpServer
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = Q_NULLPTR);

    bool start(quint16 port, const QString& serverName);
    void stop();

    // QTcpServer interface
protected:
    void incomingConnection(qintptr handle) override;

signals:
    void newClientConnected(const QString&, SessionThread* thread);
    void clientDisconnected(const QString&);
    void stopSession();

private:
    QString m_serverName;

};

#endif // SERVER_H
