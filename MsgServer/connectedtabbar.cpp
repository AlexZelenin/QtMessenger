#include "connectedtabbar.h"
#include "ui_connectedtabbar.h"

#include <QFileDialog>
#include <QDebug>
#include <QJsonDocument>
#include <QDateTime>


ConnectedTabBar::ConnectedTabBar(const QString& serverName, const QString& clientName, QWidget *parent) :
    QTabBar(parent),
    ui(new Ui::ConnectedTabBar),
    m_serverName(serverName),
    m_clientName(clientName)
{
    ui->setupUi(this);
    ui->tbtnLoadFile->setToolTip(tr("Добавить файл"));
    ui->textBrowserHistory->setOpenExternalLinks(true);

    connect(ui->btnSend, &QPushButton::clicked, this, &ConnectedTabBar::sendMessage);
    connect(ui->tbtnLoadFile, &QToolButton::clicked, this, &ConnectedTabBar::addFile);

    connect(this, &ConnectedTabBar::logMessages, this, [this](const QString& msg) {
        QString fileName = m_clientName;
        fileName.append(".txt");
        QFile fileLog(fileName);
        if (fileLog.open(QIODevice::WriteOnly | QIODevice::Append)) {
            fileLog.write(msg.toUtf8());
            fileLog.write("\n");
            fileLog.close();
        }
    });
}

ConnectedTabBar::~ConnectedTabBar()
{
    delete ui;
}

void ConnectedTabBar::sendMessage()
{
    const QString msg = ui->teMsgInput->toPlainText();
    ui->teMsgInput->clear();
    m_sendObject.insert("message", QJsonValue::fromVariant(msg));

    QJsonDocument doc(m_sendObject);

    auto data = doc.toJson();


    QString text = "%1 [%2] - %3";
    text = text.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"), m_serverName, msg);
    ui->textBrowserHistory->append(text);

    emit logMessages(text);

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

void ConnectedTabBar::addFile()
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
        data = file.readAll();
        fileJObject.insert(finfo.fileName(), QJsonValue::fromVariant(data));
        file.close();
    }
    m_fileArray.append(fileJObject);
    m_sendObject.insert("files", m_fileArray);
}

void ConnectedTabBar::reciveMessage(const QString &msg)
{
    qDebug() << "ReciveMessage" << msg;
    QJsonParseError jsonError;
    QJsonDocument doc = QJsonDocument::fromJson(msg.toUtf8(), &jsonError);
    if (jsonError.error != QJsonParseError::NoError) {
        qDebug() << "Error incoming data" << jsonError.errorString();
    }

    QJsonObject object = doc.object();

    if (object.contains("message")) {
        const QString msg = object.value("message").toString();
        QString text = "%1 [%2] - %3";
        text = text.arg(QDateTime::currentDateTime().toString("dd.MM.yyyy hh.mm.ss"), m_clientName, msg);
        ui->textBrowserHistory->append(text);
        emit logMessages(text);

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
                        return;
                    }

                    QByteArray binData = value.toObject().value(key).toString().toUtf8();
                    file.write(binData);
                    file.close();

                    QString fileLink = "<a href='%1'>%2</a>";
                    fileLink = fileLink.arg(filePath, key);
                    ui->textBrowserHistory->append(fileLink);
                }
            }
        }
    }
}
