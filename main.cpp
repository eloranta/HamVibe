#include "mainwindow.h"

#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    const QString stylePath = QCoreApplication::applicationDirPath() + "/hamvibe.qss";
    QFile styleFile(stylePath);
    if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        app.setStyleSheet(QString::fromUtf8(styleFile.readAll()));
    } else {
        qWarning() << "Failed to load stylesheet:" << stylePath;
    }
    MainWindow window;
    window.show();
    return app.exec();
}
