#include "mainwindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("HamVibe");
    QCoreApplication::setApplicationName("HamVibe");
    MainWindow window;
    window.show();
    return app.exec();
}
