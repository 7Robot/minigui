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
    static QStringList Args;
    static QString ArgPath;

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

    void AsservRotPos90();
    void AsservRotNeg90();
    void AsservRotPos360();
    void AsservRotNeg360();
    void AsservDistPos();
    void AsservDistNeg();
    void AsservJoystick(bool checked);

    void CalibUSgauche();
    void CalibUSdroite();
    void CalibUSback();
    void CalibSharpGauche();
    void CalibSharpDroite();
    void CalibSharpBack();


    void ReadIA();
    void ReadCAN();
    void ReadTheirCAN();

    void RefreshChrono();
    void CleanOurEchos();
    void CleanTheirEchos();

private:
    Ui::MainWindow *ui;

    QTcpSocket *IASocket;
    QTcpSocket *CANSocket;
    QTcpSocket *theirCANSocket;

    QString ourRobotName;
    QString theirRobotName;

    QGraphicsScene *scene;
    static const qreal scale = 320. / 3000.;
    static QPointF origin;
    QGraphicsPixmapItem *background;
    QGraphicsPixmapItem *ourRobot;
    QGraphicsPixmapItem *theirRobot;
    QGraphicsPathItem *ourEchos[4];
    QGraphicsPathItem *theirEchos[4];


    QTimer *batteryTimer;
    QTimer *chronoTimer;
    QTimer *ourEchoTimer;
    QTimer *theirEchoTimer;
    QTime matchStart;

    void RefreshRobot(QList<QByteArray> tokens, QGraphicsPixmapItem *robot);
    void RefreshEchos(QList<QByteArray> tokens, QGraphicsPixmapItem *robot, QGraphicsPathItem *echos[4], QTimer *timer);

    void BasculerVue(int vue);

    void WriteIA(QByteArray line);
    void ParseIA(QByteArray line);
    void WriteCAN(QByteArray line);
    void ParseCAN(QByteArray line);
};

#endif // MAINWINDOW_H
