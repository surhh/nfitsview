#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    int argCount = QCoreApplication::arguments().count();
    qint32 res = 1;

    if (argCount == 2)
        res = w.openFITSFileByNameFromCmdLine(QCoreApplication::arguments().at(1));

    if (res == 1)
    {
        w.showMaximized();

        return a.exec();
    }
}
