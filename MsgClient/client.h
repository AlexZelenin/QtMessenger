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

signals:
    void toServerConnected(const QString&);
    void disconnected();
    void reciveMessage(const QString&);

private:
    QTcpSocket* m_socket;
    QString m_clientName;

};

#endif // CLIENT_H
