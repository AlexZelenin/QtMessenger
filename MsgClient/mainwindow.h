#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "client.h"

#include <QJsonArray>
#include <QJsonObject>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void startClient();
    void stopClient();
    void reciveMessage(const QString& data);
    void sendMessage();
    void addFile();

signals:
    void error(const QString&);
    void sendedMessage(const QString&);

private:
    Ui::MainWindow *ui;

    QString m_serverName;
    QString m_clientName;

    Client *m_client;

    QJsonObject m_sendObject;
    QJsonArray m_fileArray;
};
#endif // MAINWINDOW_H
