#include "mainwindow.h"
#include "logger/Logger.h"

#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Logger::initLog();
    MainWindow w;
    w.show();
    return a.exec();
}
