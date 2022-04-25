#ifndef CONNECTEDTABBAR_H
#define CONNECTEDTABBAR_H

#include <QWidget>
#include <QTabBar>

#include <QJsonObject>
#include <QJsonArray>


namespace Ui {
class ConnectedTabBar;
}

class ConnectedTabBar : public QTabBar
{
    Q_OBJECT

public:
    explicit ConnectedTabBar(const QString& serverName, const QString& clientName, QWidget *parent = nullptr);
    ~ConnectedTabBar();

    QString tabName() const;

signals:
    void sendedMessage(const QString&);
    void logMessages(const QString&);

public slots:
    void sendMessage();
    void addFile();
    void reciveMessage(const QString& msg);

private:
    Ui::ConnectedTabBar *ui;

    QJsonObject m_sendObject;
    QJsonArray m_fileArray;
    QString m_serverName;
    QString m_clientName;
};

#endif // CONNECTEDTABBAR_H
