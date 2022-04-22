#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QIntValidator>

#include "connectedtabbar.h"

const char *active_text = "<p style='color: #049B00'>Активный</p>";
const char *not_active_text = "<p style='color: #FFBF00'>Не активный</p>";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_countClients(0)
{
    ui->setupUi(this);

    setWindowTitle(qApp->applicationName());

    ui->lePort->setValidator(new QIntValidator(0, 65535, this));
    ui->lblStatus->clear();
    ui->lblStatus->setText(tr(not_active_text));
    ui->btnStop->setDisabled(true);
    ui->lblCountClients->clear();

    m_server = new Server(this);

    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::startServer);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::stopServer);
    connect(m_server, &Server::newClientConnected, this, &MainWindow::newConnection);
    connect(m_server, &Server::clientDisconnected, this, &MainWindow::disconnection);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startServer()
{
    int port = ui->lePort->text().toInt();
    m_serverName = ui->leServerName->text();

    if (m_serverName.isEmpty() || !port) {
        ui->statusbar->showMessage("Поля конфигурации не заполенены. Сервер не запущен.");
        return;
    }

    ui->statusbar->clearMessage();

    if (m_server->start(port, m_serverName)) {
        ui->lblStatus->setText(tr(active_text));
        ui->btnStart->setDisabled(true);
        ui->btnStop->setDisabled(false);
        ui->lblCountClients->setText(QString::number(m_countClients));
    }
}

void MainWindow::stopServer()
{
    m_server->stop();
    ui->lblStatus->setText(tr(not_active_text));
    ui->tabWidget->clear();
    ui->btnStart->setDisabled(false);
    ui->btnStop->setDisabled(true);
    m_countClients = 0;
    ui->lblCountClients->clear();
}

void MainWindow::newConnection(const QString& clientName, SessionThread* thread)
{
    m_countClients++;
    ui->lblCountClients->setText(QString::number(m_countClients));
    ConnectedTabBar *tabBar = new ConnectedTabBar(m_serverName, clientName, ui->tabWidget);

    connect(tabBar, &ConnectedTabBar::sendedMessage, thread,
            &SessionThread::sendMessage, Qt::QueuedConnection);
    connect(thread, &SessionThread::recivedMessage, tabBar,
            &ConnectedTabBar::reciveMessage, Qt::QueuedConnection);

    int index = ui->tabWidget->addTab(tabBar, clientName);
    m_tabs.insert(clientName, index);
}

void MainWindow::disconnection(const QString& clientName)
{
    m_countClients ? m_countClients-- : m_countClients = 0;

    ui->lblCountClients->setText(QString::number(m_countClients));
    int index = m_tabs.value(clientName);
    ui->tabWidget->removeTab(index);
}

