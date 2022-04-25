#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QRegExpValidator>
#include <QIntValidator>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QFile>
#include <QFileDialog>
#include <QDir>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /*
     * Регулярным выражением ограничиваем ввод символов для заполнения IP адреса
    */
    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    QRegExp ipRegex ("^" + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange + "$");


    ui->leIPAddress->setValidator(new QRegExpValidator(ipRegex, this));
    ui->lePort->setValidator(new QIntValidator(0, 65535, this));

    ui->btnStop->setDisabled(true);

    ui->textBrowserHistory->setOpenExternalLinks(true);

    m_client = new Client(this);


    connect(ui->btnStart, &QPushButton::clicked, this, &MainWindow::startClient);
    connect(ui->btnStop, &QPushButton::clicked, this, &MainWindow::stopClient);

    /*
     * Обработка покдлючения к серверу в случае успешного подключения
     * И вывод сообщения о пользователю
    */
    connect(m_client, &Client::toServerConnected, this, [this](const QString& msg){
        m_serverName = msg;
        QString text = "<span>%1 [System] - Вы успешно подключились к серверу '%2'</span>";
        text = text.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"), msg);
        ui->textBrowserHistory->append(text);
    });

    /*
     * Вывод ошибок пользователю
    */
    connect(this, &MainWindow::error, ui->textBrowserHistory, &QTextBrowser::append);


    connect(m_client, &Client::reciveMessage, this, &MainWindow::reciveMessage);
    connect(ui->btnSend, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(ui->tbtnLoadFile, &QToolButton::clicked, this, &MainWindow::addFile);

    connect(this, &MainWindow::sendedMessage, m_client, &Client::sendMessage);

    connect(m_client, &Client::connectedState, this, &MainWindow::connectedStateHandle);
    connect(m_client, &Client::connectionRefuse, this, &MainWindow::connectingRefuseHandle);

    connect(m_client, &Client::networkError, this, &MainWindow::errorHandle);
    connect(m_client, &Client::hostNotFound, this, &MainWindow::errorHandle);
    connect(m_client, &Client::socketTimeOut, this, &MainWindow::errorHandle);

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
 * Слот обработки запуска клиента
*/
void MainWindow::startClient()
{
    ui->statusbar->clearMessage();

    /*
     * Получаем введенные данные
    */
    const QString hostName = ui->leIPAddress->text();
    const int port = ui->lePort->text().toInt();
    m_clientName = ui->leClientName->text();

    if (hostName.isEmpty() || m_clientName.isEmpty() || !port) {
        ui->statusbar->showMessage("Вы не заполнили конфигурацию.");
        return;
    }

    /*
     * Подключаемся к серверу, в случае успеха переключаем дисэйблим кнопку старт и активируем кнопку стоп
     * в противном случае сообщаем о не успешном подключении
    */
    if (m_client->clientConnect(hostName, port, m_clientName)) {
        ui->btnStart->setDisabled(true);
        ui->btnStop->setDisabled(false);
    } else {
        QString err = "<span>%1 [System] - Соединение с сервером не установелено</span>";
        err = err.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"));
        emit error(err);
    }
}

void MainWindow::stopClient()
{
    /*
     * Отключение клиента. сообщаем пользователю об остановке
    */
    m_client->clientDisconnect();
    ui->btnStart->setDisabled(false);
    ui->btnStop->setDisabled(true);
    QString text = "%1 [%2] - %3";
    text = text.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"), "System", "Соединение остановлено");
    ui->textBrowserHistory->append(text);
}

/*
 * Метод обработки полученных сообщений
*/
void MainWindow::reciveMessage(const QString &data)
{
    /*
     * Переводим входящие данные в тип Json и проверяем структуру на валидность
    */
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "Error incoming data" << jsonError.errorString();
    }

    QJsonObject object = doc.object();

    /*
     * Если есть ключ 'message', то получаем его значение и выводим его на экран истории переписки
     * в формате dd.MM.yyyy hh.mm.ss [%servername%] - текст сообщения
    */
    if (object.contains("message")) {
        const QString msg = object.value("message").toString();
        QString text = "<span>%1 [%2] - %3</span>";
        text = text.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"), m_serverName, msg);
        ui->textBrowserHistory->append(text);

        /*
         * Дальше проверяем если есть есть ключ 'files'
         * Получаем его значение в QJsonArray
         * Проверяем есть ли папка в корне программы Downloads, если нету создаем
         * В цикле проходим по всем значения массива Json
         * Так как массив состоит из значений других Json типа { 'имя файля': 'данные файла' }
         * То получив ключи создаем еще цикл в которм полчаем имя файла и по имени достаем его данные
         * далее сохранем файл в папку Downloads
         * На экран истории сообщений выводи ссылку на файл.
         *  !!! На ОС Windows ссылка открывается в системном браузере, а на ОС Ubuntu почему то открывает файл
         *  в самом окне истории сообщений (пока не разобрался). Ну и так как кодеки не установлены в приложении, выводит кракозябры.
         *  Если файл открыть в ручную из директории то выводит все отклично.
        */
        if (object.contains("files")) {
            QJsonArray filesArray = object.value("files").toArray();

            QString path = QDir::currentPath();
            path = path.append("/Downloads");

            if (!QDir(path).exists())
                QDir().mkdir(path);

            foreach(const QJsonValue& value, filesArray) {

                foreach(const QString& key, value.toObject().keys()) {
                    QString filePath = path;
                    filePath.append("/").append(key);

                    QFile file(filePath);
                    if (!file.open(QIODevice::WriteOnly)) {
                        emit error("Возникла ошибка при создании файла");
                        return;
                    }

                    QByteArray binData = QByteArray::fromHex(value.toObject().value(key).toString().toUtf8());
                    file.write(binData);
                    file.close();

                    QString fileLink = "<a href=\"%1\">%2</a>";
                    fileLink = fileLink.arg(filePath, key);
                    ui->textBrowserHistory->append(fileLink);
                }
            }
        }
    }
}


/*
 * Отправка сообщений
 * Получем текст из поля воода текста и формируем сообщение с датой и временем отправки
 * Далее записываем в окно истории сообщений
 * и отправляем данные на сервер
*/
void MainWindow::sendMessage()
{
    const QString msg = ui->textEdit->toPlainText();
    ui->textEdit->clear();

    m_sendObject.insert("message", QJsonValue::fromVariant(msg));

    QJsonDocument doc(m_sendObject);

    auto data = doc.toJson();

    QString text = "<span>%1 [%2] - %3</span>";
    text = text.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"), m_clientName, msg);
    ui->textBrowserHistory->append(text);

    if (m_sendObject.contains("files")) {
        foreach(const QJsonValue& key, m_fileArray) {
            QStringList keys = key.toObject().keys();
            foreach(const QString& filename, keys) {
                ui->textBrowserHistory->append(filename);
            }
        }
        ui->lblFileInfo->clear();
    }

    emit sendedMessage(data);

    while(m_fileArray.count()) {
         m_fileArray.pop_back();
    }

    foreach(const QString& key, m_sendObject.keys()) {
        m_sendObject.remove(key);
    }
}

void MainWindow::addFile()
{
    QString filePath = QFileDialog::getOpenFileName(Q_NULLPTR, "Добавить файл", "", "*.txt");
    QFile file(filePath);
    QFileInfo finfo(file);
    QString text = ui->lblFileInfo->text();
    text.append(" ").append(finfo.fileName());
    ui->lblFileInfo->setText(text);

    QJsonObject fileJObject;

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data;
        data.clear();
        data = file.readAll();
        fileJObject.insert(finfo.fileName(), QJsonValue::fromVariant(data.toHex()));
        file.close();
    }
    m_fileArray.append(fileJObject);
    m_sendObject.insert("files", m_fileArray);
}

void MainWindow::connectedStateHandle()
{
    ui->textEdit->setDisabled(false);
    ui->btnSend->setDisabled(false);
}

void MainWindow::connectingRefuseHandle()
{
    ui->textEdit->setDisabled(true);
    ui->btnSend->setDisabled(true);
}

void MainWindow::errorHandle(const QString &error)
{
    QString text = "<span>%1 [%2] - %3</span>";
    text = text.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"), "System", error);
    ui->textBrowserHistory->append(text);
}
