#include "mainwindow.h"

#ifdef _DEBUG
#include "logger/Logger.h"
#endif



#include <QApplication>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#if _DEBUG
    Logger::initLog();
#endif
    MainWindow w;
    w.show();
    return a.exec();
}
