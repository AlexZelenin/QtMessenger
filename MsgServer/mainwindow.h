#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "server.h"
#include "sthread.h"

#include <QHash>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


signals:
    void closeSession(QThread*);

public slots:
    void startServer();
    void stopServer();
    void newConnection(const QString& clientName, SessionThread* thread);
    void disconnection(const QString& clientName);

private:
    Ui::MainWindow *ui;

    Server *m_server;
    unsigned int m_countClients;

    /*
     * QHash нужен для быстрого поиска значения по ключу
    */
    QHash<QString, WId> m_tabs;
    QString m_serverName;

};
#endif // MAINWINDOW_H
