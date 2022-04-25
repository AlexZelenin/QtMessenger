#ifndef STHREAD_H
#define STHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QTimer>

class SessionThread : public QObject
{
    Q_OBJECT

public:
    SessionThread(int socketDescriptor, const QString& serverName, QObject* parent = Q_NULLPTR);
    ~SessionThread();

signals:
    void error(QTcpSocket::SocketError socketError);
    void newClientConnected(const QString&, SessionThread*);
    void clientDisconnected(const QString&);
    void recive(const QString&);
    void send(const QString&);

public slots:
    void runThread();
    void readyRead();
    void disconnected();
    void stop();
    void errorConnection(QAbstractSocket::SocketError error);
    void connectionState(QAbstractSocket::SocketState state);

    void sendMessage(const QString& msg);

private:
    int m_socketDescriptor;
    QString m_serverName;
    QString m_clientName;
    QTcpSocket *m_socket;
    quint16 m_nextBlockSize;

    QByteArray m_data;

    QTimer *m_timer;
};

#endif // STHREAD_H
