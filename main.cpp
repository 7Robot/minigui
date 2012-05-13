#include <QtGui/QApplication>
#include <QThread>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    MainWindow::ArgPath = argv[0];
    for(int i = 1; i < argc; i++) { // Sauvegarde des arguments.
        MainWindow::Args << argv[i];
    }

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
