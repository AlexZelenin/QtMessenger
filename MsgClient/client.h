#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>


class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = nullptr);

    bool clientConnect(const QString& hostName, const int& port, const QString& clientName);
    void clientDisconnect();


public slots:
    void connected();
    void readyRead();
    void sendMessage(const QString& msg);
    void errorConnection(QAbstractSocket::SocketError error);
    void connectionState(QAbstractSocket::SocketState state);

signals:
    void toServerConnected(const QString&);
    void disconnected();
    void reciveMessage(const QString&);
    void connectedState();
    void connectionRefuse();

private:
    QTcpSocket* m_socket;
    QString m_clientName;

    QString m_hostName;
    quint16 m_port;
    quint16 m_nextBlockSize;

    QByteArray m_data;

};

#endif // CLIENT_H
