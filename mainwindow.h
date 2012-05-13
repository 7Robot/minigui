#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QTimer>
#include <QTime>


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
    void FileReboot();
    void FileHalt();
    void FileRestart();
    void FileQuit();

    void OdoRouge();
    void OdoViolet();
    void OdoRecalage();
    void OdoRequest();
    void OdoMute();
    void OdoUnmute();

    void Cmd1();
    void Cmd2();
    void Cmd3();
    void Cmd4();
    void Cmd5();

    void CalibUSgauche();
    void CalibUSdroite();
    void CalibUSback();


    void ReadIA();
    void ReadCAN();

    void VueMessages();
    void VuePlateau();
    void VueMatch();


    void RefreshBattery();
    void RefreshChrono();

private:
    Ui::MainWindow *ui;
    QTcpSocket *SocketIA;
    QTcpSocket *SocketCAN;

    QGraphicsScene *scene;
    QGraphicsPixmapItem *background;
    QGraphicsPixmapItem *robot;
    QGraphicsPathItem *echos[4];
    static const qreal scale = 320. / 3000.;
    static QPointF origin;

    QTimer *batteryTimer;
    QTimer *chronoTimer;
    QTime matchStart;

    void WriteIA(QByteArray line);
    void ParseIA(QByteArray line);
    void WriteCAN(QByteArray line);
    void ParseCAN(QByteArray line);
    void RefreshRobot(int x, int y, int theta);

    void Vue(int vue);
};

#endif // MAINWINDOW_H
