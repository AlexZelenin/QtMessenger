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

    connect(ui->lePort, &QLineEdit::textChanged, this, [this](){
        if (ui->lePort->text().toInt() > 65535)
            ui->lePort->setText(QString::number(65535));
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*
 * Метод запуска сервера
*/
void MainWindow::startServer()
{
    /*
     * Получаем введенные данные конфигурации
     * Порт и имя сервера
    */
    int port = ui->lePort->text().toInt();
    m_serverName = ui->leServerName->text();

    /*
     * Проверяем если поля не заполнены, то выходим из метода и сообщаем пользователю
     * о том что поля не заполнены
    */
    if (m_serverName.isEmpty() || !port) {
        ui->statusbar->showMessage("Поля конфигурации не заполенены. Сервер не запущен.");
        return;
    }

    ui->statusbar->clearMessage();

    /*
     * Запускаем сервер. Если сервер запустился, переключаем статус в активный.
    */
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

/*
 * Новое подключение. Функция принимает имя поключившегося клиента и его поток сессии
*/
void MainWindow::newConnection(const QString& clientName, SessionThread* thread)
{
    /*
     * Увеличиваем счетчик поключений на 1
     * и информируем в разделе статус.
    */
    m_countClients++;
    ui->lblCountClients->setText(QString::number(m_countClients));

    /*
     * Создаем новую вкладку для нового подключения
    */
    ConnectedTabBar *tabBar = new ConnectedTabBar(m_serverName, clientName, ui->tabWidget);

    /**/
    connect(tabBar, &ConnectedTabBar::sendedMessage, thread,
            &SessionThread::sendMessage, Qt::QueuedConnection);
    connect(thread, &SessionThread::recive, tabBar,
            &ConnectedTabBar::reciveMessage, Qt::QueuedConnection);

    /*
     * Получаем ID виджета для дальнейшего обращения именно к этой вкладке
     * и добавляем его в QHash где ключ это имя клиента, значение WId
    */
    WId wid = tabBar->winId();
    m_tabs.insert(clientName, wid);

    ui->tabWidget->addTab(tabBar, clientName);
}

/*
 * Мотод отключения клиента от сервера
*/
void MainWindow::disconnection(const QString& clientName)
{
    /*
     *Уменьшаем счетчик на 1. Перед этим проверяем что бы он был больше 0
    */
    if (m_countClients)
        m_countClients--;

    ui->lblCountClients->setText(QString::number(m_countClients));

    /*
     * Получаем WId по имени клиента
     * и удаляем его из QHash
    */
    WId wid = m_tabs.value(clientName);
    m_tabs.remove(clientName);

    /*
     * Находим нужный виджет вкладки и помечаем его на удаление
    */
    QWidget* widget = ui->tabWidget->find(wid);
    widget->deleteLater();
}
