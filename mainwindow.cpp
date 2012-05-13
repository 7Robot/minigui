#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QtCore/qmath.h>
#include <QDebug>
#include <QMessageBox>
#include <QPen>
#include <QBrush>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QHostInfo>
#include <QProcess>


QString MainWindow::ArgPath;
QStringList MainWindow::Args; // Les arguments de main() à peu de chose près.

QPointF MainWindow::origin(160, 106.5);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent, Qt::FramelessWindowHint),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString host = QHostInfo::localHostName();
    if(host != "gros") {
        ourRobotName = "petit"; // Par défaut
        theirRobotName = "gros";
    }
    else {
        ourRobotName = "gros";
        theirRobotName = "petit";
    }

    BasculerVue(2);

    // Préparation de la vue Plateau. */
    QPixmap plateauPix = QPixmap(":/pics/plateau.png");
    scene = new QGraphicsScene(0, 0, plateauPix.width(), plateauPix.height(), this);
    background = scene->addPixmap(plateauPix);

    QPixmap ourRobotPix = QPixmap(":/pics/" + ourRobotName + ".png");
    ourRobot = scene->addPixmap(ourRobotPix);
    ourRobot->setTransformOriginPoint(ourRobotPix.width() / 2., ourRobotPix.height() / 2.);
    ourRobot->setVisible(false);
    ourRobot->setPos(0, 0); // Valeur initiale sur laquelle se basent les echos.

    QPixmap theirRobotPix = QPixmap(":/pics/" + theirRobotName + ".png");
    theirRobot = scene->addPixmap(theirRobotPix);
    theirRobot->setTransformOriginPoint(theirRobotPix.width() / 2., theirRobotPix.height() / 2.);
    theirRobot->setVisible(false);
    theirRobot->setPos(0, 0); // Valeur initiale sur laquelle se basent les echos.

    QPen ourPen   = QPen(QColor(255, 0, 0));
    QPen theirPen = QPen(QColor(0, 255, 0));
    ourPen.setWidth(4);
    theirPen.setWidth(4);

    for(int i = 0; i < 4; i++) {
        ourEchos[i] = new QGraphicsPathItem();
        ourEchos[i]->setPen(ourPen);
        ourEchos[i]->setPos(ourRobot->transformOriginPoint());
        scene->addItem(ourEchos[i]);

        theirEchos[i] = new QGraphicsPathItem();
        theirEchos[i]->setPen(theirPen);
        theirEchos[i]->setPos(theirRobot->transformOriginPoint());
        scene->addItem(theirEchos[i]);
    }

    ui->plateau->setScene(scene);
    ////////////////////////////////////


    /* Préparation du réseau. */
    CANSocket = new QTcpSocket(this);
    IASocket = new QTcpSocket(this);
    theirCANSocket = new QTcpSocket(this);

    connect(CANSocket,       SIGNAL(readyRead()), this, SLOT(ReadCAN()));
    connect(IASocket,        SIGNAL(readyRead()), this, SLOT(ReadIA()));
    connect(theirCANSocket,  SIGNAL(readyRead()), this, SLOT(ReadTheirCAN()));

    int CANport, IAport, theirCANport;
    if(ourRobotName == "petit") {
        CANport = 7773;
        IAport  = 7774;
        theirCANport = 7777;
    }
    else {
        CANport = 7777;
        IAport  = 7778;
        theirCANport = 7773;
    }
    CANSocket->connectToHost(ourRobotName, CANport);
    IASocket->connectToHost(ourRobotName,  IAport);
    theirCANSocket->connectToHost(theirRobotName, theirCANport);

    CANSocket->waitForConnected();
    IASocket->waitForConnected();
    ////////////////////////////////////


    cleanEchosTimer = new QTimer(this);
    cleanEchosTimer->setSingleShot(true);
    connect(cleanEchosTimer, SIGNAL(timeout()), this, SLOT(CleanEchos()));

    chronoTimer = new QTimer(this);
    connect(chronoTimer, SIGNAL(timeout()), this, SLOT(RefreshChrono()));

    batteryTimer = new QTimer(this);
    connect(batteryTimer, SIGNAL(timeout()), this, SLOT(FileBattery()));

    batteryTimer->start(30000);
    FileBattery(); // Actualise le niveau batterie toutes les 30 secondes.

    WriteIA("MESSAGE minigui started on " + host.toUtf8() + " for " + ourRobotName.toUtf8() + ".");
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::BasculerVue(int vue)
{
    ui->messages->setVisible(vue == 1);
    ui->plateau ->setVisible(vue == 2);
    ui->match   ->setVisible(vue == 3);

    ui->actionMessages->setChecked(vue == 1);
    ui->actionPlateau ->setChecked(vue == 2);
    ui->actionMatch   ->setChecked(vue == 3);
}


void MainWindow::RefreshChrono()
{
    int remaining = (90 - matchStart.elapsed() / 1000);
    ui->chrono->display(remaining);
}


void MainWindow::RefreshRobot(QList<QByteArray> tokens, QGraphicsPixmapItem *robot)
{
    int x = tokens.at(2).toInt();
    int y = tokens.at(3).toInt();
    int theta = tokens.at(4).toInt();

    robot->setVisible(true);
    robot->setRotation(180 - theta / 100.);
    robot->setPos(origin + QPointF(-x, y) * scale - robot->transformOriginPoint());
}


void MainWindow::RefreshEchos(QList<QByteArray> tokens, QGraphicsPixmapItem *robot, QGraphicsPathItem *echos[4])
{
    //if(robot->isVisible() == false)
    //{
    //    return;
    //}

    for(int i = 0, j = 2; i < 4; i++) {
        if(j + 1 < tokens.count()) {
            qreal dist  = tokens.at(j++).toInt() * 10 * scale;
            qreal angle = tokens.at(j++).toInt() - robot->rotation() - 90;
            qreal span = 360. * 80. /* rayon apparent */ * scale / dist / 2 / 3.14159265f;
            QRectF rect = QRectF(robot->x() - dist, robot->y() - dist, dist * 2, dist * 2);

            QPainterPath path = QPainterPath();
            path.arcMoveTo(rect, angle - span / 2);
            path.arcTo(rect, angle - span / 2, span);

            echos[i]->setPath(path);

            echos[i]->show();
        }
        else {
            echos[i]->hide();
        }
    }

    cleanEchosTimer->start(1000); // On efface les vieux echos automatiquement.
}

void MainWindow::CleanEchos()
{
    for(int i = 0; i < 3; i++) {
        ourEchos[i]->hide();
    }
}

void MainWindow::ReadCAN()
{
    if(!CANSocket->canReadLine())
        return; // Not a full line yet.

    MainWindow::ParseCAN(CANSocket->readLine().simplified());
}

void MainWindow::WriteCAN(QByteArray line)
{
    CANSocket->write(line + "\n");
    ParseCAN(line); // Echo back to refresh view.
}

void MainWindow::ParseCAN(QByteArray line)
{
    ui->historyCAN->appendPlainText(line);
    line = line.toUpper();
    QList<QByteArray> tokens = line.split(' ');

    if(tokens.size() == 5 && tokens.at(0) == "ODO" && (tokens.at(1) == "POS" || tokens.at(1) == "SET")) {
        // Actualisation du robot.
        RefreshRobot(tokens, ourRobot);
    }
    else if(tokens.size() > 2 && tokens.at(0) == "TURRET" && tokens.at(1) == "ANSWER") {
        // Actualisation du radar.
        RefreshEchos(tokens, ourRobot, ourEchos);
    }
    else if(tokens.size() == 3 && tokens.at(0) == "BATTERY" && tokens.at(1) ==  "ANSWER") {
        QByteArray voltage = tokens.at(2);
        voltage.append(" V");
        ui->actionBattery->setText(voltage);
    }
}




void MainWindow::ReadIA()
{
    if(!IASocket->canReadLine())
        return;

    MainWindow::ParseIA(IASocket->readLine().simplified());
}

void MainWindow::WriteIA(QByteArray line)
{
    IASocket->write(line + "\n");
    ParseIA(line); // Echo back to refresh view.
}

void MainWindow::ParseIA(QByteArray line)
{
    ui->historyIA->appendPlainText(line);
    line = line.toUpper();
    QList<QByteArray> tokens = line.split(' ');

    if(tokens.size() == 1 && tokens.at(0) == "START") {
        chronoTimer->start(1000);
        matchStart.start();
        RefreshChrono();
    }
    else if(tokens.size() == 1 && tokens.at(0) == "STOP") {
        if(chronoTimer->isActive())
            RefreshChrono();
        chronoTimer->stop();
    }
    else if(tokens.size() == 2 && tokens.at(0) == "VUE") {
        BasculerVue(tokens.at(1).toInt());
    }
}


void MainWindow::ReadTheirCAN()
{
    if(!theirCANSocket->canReadLine())
        return;

    QByteArray line = theirCANSocket->readLine().simplified().toUpper();
    QList<QByteArray> tokens = line.split(' ');

    if(tokens.size() == 5 && tokens.at(0) == "ODO" && (tokens.at(1) == "POS" || tokens.at(1) == "SET")) {
        // Actualisation du robot.
        RefreshRobot(tokens, theirRobot);
    }
    else if(tokens.size() > 2 && tokens.at(0) == "TURRET" && tokens.at(1) == "ANSWER") {
        // Actualisation du radar.
        RefreshEchos(tokens, theirRobot, theirEchos);
    }
}




////////////////////////////////////////////////////////////////////////
/////////////////////////////// Menus //////////////////////////////////
////////////////////////////////////////////////////////////////////////



void  MainWindow::FileBattery()
{
    WriteCAN("BATTERY REQUEST");
}

void  MainWindow::FileReset()
{
    WriteCAN("RESET");
}

void MainWindow::FileReboot(){
    QMessageBox msgBox;
    msgBox.setText(QString::fromUtf8("Redémarrer l'ARM ?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if(msgBox.exec() == QMessageBox::Ok) {
        WriteIA("MESSAGE minigui reboot");
        qDebug() << system("reboot");
    }
}

void MainWindow::FileHalt()
{
    QMessageBox msgBox;
    msgBox.setText(QString::fromUtf8("Éteindre l'ARM ?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if(msgBox.exec() == QMessageBox::Ok) {
        WriteIA("MESSAGE minigui halt");
        qDebug() << system("halt");
    }
}

void MainWindow::FileRestart()
{
    WriteIA("MESSAGE minigui restart");

    QProcess proc;
    proc.startDetached(ArgPath, Args); // On relance le même processus.

    QApplication::exit();
}

void MainWindow::FileQuit()
{
    WriteIA("MESSAGE minigui quit");
    QApplication::exit();
}





void MainWindow::VueMessages()
{
    BasculerVue(1);
}

void MainWindow::VuePlateau()
{
    BasculerVue(2);
}

void MainWindow::VueMatch()
{
    BasculerVue(3);
}

void MainWindow::TourelleOn()
{
    WriteCAN("TURRET ON");
}

void MainWindow::TourelleOff()
{
    WriteCAN("TURRET OFF");
}





void MainWindow::Cmd1()
{
    WriteIA("CMD1");
}

void MainWindow::Cmd2()
{
    WriteIA("CMD2");
}

void MainWindow::Cmd3()
{
    WriteIA("CMD3");
}

void MainWindow::Cmd4()
{
    WriteIA("CMD4");
}

void MainWindow::Cmd5()
{
    WriteIA("CMD5");
}

void MainWindow::InitRed()
{
    WriteIA("INIT RED");
    ui->initRed->hide();
    ui->initViolet->hide();
}

void MainWindow::InitViolet()
{
    WriteIA("INIT VIOLET");
    ui->initRed->hide();
    ui->initViolet->hide();
}

void MainWindow::RestartIA()
{
    QMessageBox msgBox;
    msgBox.setText("Relancer l'IA ?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if(msgBox.exec() == QMessageBox::Ok) {
        WriteIA("restart ia");

        QProcess kill;
        kill.start("killall python"); // Ne marche pas si l'IA tourne à distance.
        qDebug() << "killall: " << kill.waitForFinished(1000);

        QProcess ia;
        ia.startDetached("/home/ia/ia.py", QStringList(ourRobotName));

        // Affiche à nouveau le choix de coté.
        ui->initRed->show();
        ui->initViolet->show();
    }
}





void MainWindow::OdoRouge()
{
    WriteCAN("ODO SET 1250 750 18000");
}

void MainWindow::OdoViolet()
{
    WriteCAN("ODO SET -1250 750 0");
}

void MainWindow::OdoPalmier()
{
    WriteCAN("ODO SET 0 0 27000");
}

void MainWindow::OdoRecalage()
{
    WriteIA("POSITIONING");
}

void MainWindow::OdoRequest()
{
    WriteCAN("ODO REQUEST");
}

void MainWindow::OdoUnmute()
{
    WriteCAN("ODO UNMUTE");
}

void MainWindow::OdoMute()
{
    WriteCAN("ODO MUTE");
}





void MainWindow::AsservRotPos90()
{
    WriteCAN("ASSERV ROT 9000");
}

void MainWindow::AsservRotNeg90()
{
    WriteCAN("ASSERV ROT -9000");
}

void MainWindow::AsservRotPos360()
{
    WriteCAN("ASSERV ROT 36000");
}

void MainWindow::AsservRotNeg360()
{
    WriteCAN("ASSERV ROT -36000");
}

void MainWindow::AsservDistPos()
{
    WriteCAN("ASSERV DIST 10000");
}

void MainWindow::AsservDistNeg()
{
    WriteCAN("ASSERV DIST -10000");
}

void MainWindow::AsservJoystick(bool checked)
{
    if(checked) {

    }
    else {

    }
}





void MainWindow::CalibUSgauche()
{
    WriteIA("CALIBRATE 1");
}

void MainWindow::CalibUSdroite()
{
    WriteIA("CALIBRATE 2");
}

void MainWindow::CalibUSback()
{
    WriteIA("CALIBRATE 8");
}

void MainWindow::CalibSharpGauche()
{
    WriteIA("CALIBRATE 5");
}

void MainWindow::CalibSharpDroite()
{
    WriteIA("CALIBRATE 4");
}

void MainWindow::CalibSharpBack()
{
    WriteIA("CALIBRATE 3");
}
