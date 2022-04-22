#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDebug>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QApplication::setApplicationName("Message Server");

    QFile style(":/styles");

    if (style.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(style.readAll());
    } else {
        qDebug() << style.errorString();
    }

    MainWindow w;
    w.show();
    return a.exec();
}
