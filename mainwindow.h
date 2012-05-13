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

    void VueMessages();
    void VuePlateau();
    void VueMatch();
    void TourelleOn();
    void TourelleOff();

    void Cmd1();
    void Cmd2();
    void Cmd3();
    void Cmd4();
    void Cmd5();
    void InitRed();
    void InitViolet();
    void RestartIA();

    void OdoRouge();
    void OdoViolet();
    void OdoPalmier();
    void OdoRecalage();
    void OdoRequest();
    void OdoUnmute();
    void OdoMute();

    void AsservRotPos();
    void AsservRotNeg();
    void AsservDistPos();
    void AsservDistNeg();

    void CalibUSgauche();
    void CalibUSdroite();
    void CalibUSback();
    void CalibSharpGauche();
    void CalibSharpDroite();
    void CalibSharpBack();


    void ReadIA();
    void ReadCAN();

    void RefreshChrono();

private:
    Ui::MainWindow *ui;

    QTcpSocket *SocketIA;
    QTcpSocket *SocketCAN;

    QString robotName;

    QGraphicsScene *scene;
    static const qreal scale = 320. / 3000.;
    static QPointF origin;
    QGraphicsPixmapItem *background;
    QGraphicsPixmapItem *robot;
    QGraphicsPathItem *echos[4];


    QTimer *batteryTimer;
    QTimer *chronoTimer;
    QTime matchStart;

    void RefreshRobot(int x, int y, int theta);
    void RefreshEchos(QList<QByteArray> tokens);

    void BasculerVue(int vue);

    void WriteIA(QByteArray line);
    void ParseIA(QByteArray line);
    void WriteCAN(QByteArray line);
    void ParseCAN(QByteArray line);
};

#endif // MAINWINDOW_H
