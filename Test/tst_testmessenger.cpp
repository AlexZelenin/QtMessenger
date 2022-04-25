#include <QtTest>
#include <QCoreApplication>

// add necessary includes here

#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>

class TestMessenger : public QObject
{
    Q_OBJECT

public:
    TestMessenger();
    ~TestMessenger();

private slots:
    void test_case1();
    void test_case2();
    void test_case3();

private:
    void sendMessage(const QString &msg);
    QString readyRead() const;

private:
    QTcpSocket *m_socket;
    QByteArray m_data;

};

TestMessenger::TestMessenger()
{

}

TestMessenger::~TestMessenger()
{

}

void TestMessenger::test_case1()
{
    m_socket = new QTcpSocket;
    m_socket->connectToHost("127.0.0.1", 8000);

    QVERIFY(m_socket->isOpen());
    m_socket->close();
    m_socket->deleteLater();
}

void TestMessenger::test_case2()
{
    m_socket = new QTcpSocket;
    m_socket->connectToHost("127.0.0.1", 8000);

    QJsonObject obj;
    obj.insert("clientname", "Test1");
    QJsonDocument doc(obj);
    sendMessage(doc.toJson());

    QString data = readyRead();

    QVERIFY(!data.isEmpty());

    m_socket->close();
    m_socket->deleteLater();

}

void TestMessenger::test_case3()
{
    m_socket = new QTcpSocket;
    m_socket->connectToHost("127.0.0.1", 8000);

    QJsonObject obj;
    obj.insert("clientname", "Test1");
    QJsonDocument doc(obj);
    sendMessage(doc.toJson());

    QString data = readyRead();

    QJsonParseError jError;
    QJsonDocument docResponse = QJsonDocument::fromJson(data.toUtf8(), &jError);

    if (jError.error != QJsonParseError::NoError) {
        QString error = QString("Error parse incoming data: %1").arg(jError.errorString());
        QFAIL(error.toUtf8());
    }

    QJsonObject objResponse = docResponse.object();

    QVERIFY(objResponse.contains("servername"));

    m_socket->close();
    m_socket->deleteLater();
}

void TestMessenger::sendMessage(const QString &msg)
{
    m_data.clear();
    QDataStream out(&m_data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << quint16(0) << msg;
    out.device()->seek(0);
    out << quint16(m_data.size() - sizeof(quint16));
    m_socket->write(m_data);
}

QString TestMessenger::readyRead() const
{
    QString data;
    quint16 nextBlockSize = 0;
    while(m_socket->waitForReadyRead(3000)) {
        QDataStream stream(m_socket);
        stream.setVersion(QDataStream::Qt_5_15);

        while(true) {
            if (!nextBlockSize) {
                if (m_socket->bytesAvailable() < sizeof(quint16)) {
                    break;
                }
                stream >> nextBlockSize;
            }

            if (m_socket->bytesAvailable() < nextBlockSize) {
                break;
            }

            stream >> data;
            nextBlockSize = 0;
        }
    }

    return data;
}

QTEST_MAIN(TestMessenger)

#include "tst_testmessenger.moc"
