#include <QtGui/QApplication>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    char ** Argv = new char*[argc + 1];
    for(int i = 0; i < argc; i++) {
        Argv[i] = argv[i];
    }
    Argv[argc] = 0;
    MainWindow::Argv = Argv;

    return a.exec();
}
