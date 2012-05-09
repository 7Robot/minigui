#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

namespace Ui {
    class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    static char **Argv;

public slots:
    void FileBattery();
    void FileReset();
    void FileRestart();
    void FileQuit();
    void FileReboot();
    void FileHalt();

    void OdoMute();
    void OdoUnmute();
    void OdoRouge();
    void OdoViolet();

    void Cmd1();
    void Cmd2();
    void Cmd3();
    void Cmd4();
    void Cmd5();

    void ReadIA();
    void ReadCAN();

private:
    Ui::MainWindow *ui;
    QTcpSocket *SocketIA;
    QTcpSocket *SocketCAN;

    void ParseCAN(QByteArray line);
    void WriteBackCAN(QByteArray line);
    void RefreshRobot(int x, int y, int theta);
};

#endif // MAINWINDOW_H
