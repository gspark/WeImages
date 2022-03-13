#include "mainwindow.h"
#include "application.h"

#include <QCommandLineParser>

int main(int argc, char *argv[])
{
//    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setOrganizationName("qView");
    QCoreApplication::setApplicationName("qView");
    QCoreApplication::setApplicationVersion(QString::number(VERSION));
    Application app(argc, argv);

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QObject::tr("file"), QObject::tr("The file to open."));
    parser.process(app);

    auto *window = Application::newWindow();
    if (!parser.positionalArguments().isEmpty())
        Application::openFile(window, parser.positionalArguments().constFirst(), true);

    return QApplication::exec();
}
