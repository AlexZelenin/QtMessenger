#ifndef STHREAD_H
#define STHREAD_H

#include <QThread>
#include <QTcpSocket>

class SessionThread : public QObject
{
    Q_OBJECT
public:
    SessionThread(int socketDescriptor, const QString& serverName, QObject* parent = Q_NULLPTR);

    // QThread interface
protected:
    //void run() override;


signals:
    void error(QTcpSocket::SocketError socketError);
    void newClientConnected(const QString&, SessionThread*);
    void clientDisconnected(const QString&);
    void recivedMessage(const QString&);

public slots:
    void runThread();
    void readyRead();
    void disconnected();
    void stop();
    void errorConnection(const QAbstractSocket::SocketError&);

    void sendMessage(const QString& msg);

private:
    int m_socketDescriptor;
    QString m_serverName;
    QString m_clientName;
    QTcpSocket *m_socket;
};

#endif // STHREAD_H
