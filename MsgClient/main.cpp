#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile style(":/styles");

    if (style.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet(style.readAll());
    } else {
        qDebug() << style.errorString();
    }

    MainWindow *w = new MainWindow;
    w->show();
    return a.exec();
}
