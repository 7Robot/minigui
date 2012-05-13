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

#include <unistd.h> // Pour les execs sur ARM.



char ** MainWindow::Argv; // Les arguments de main() à peu de chose près.

QPointF MainWindow::origin(160, 106.5);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent, Qt::FramelessWindowHint),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString host = QHostInfo::localHostName();
    if(host == "gros")
        robotName = host;
    else
        robotName = "petit"; // par défaut

    BasculerVue(2);

    // Préparation de la vue Plateau. */
    QPixmap plateauPix = QPixmap(":/pics/plateau.png");
    scene = new QGraphicsScene(0, 0, plateauPix.width(), plateauPix.height(), this);
    background = scene->addPixmap(plateauPix);

    QPixmap robotPix = QPixmap(":/pics/" + robotName + ".png");
    robot = scene->addPixmap(robotPix);
    robot->setTransformOriginPoint(robotPix.width() / 2., robotPix.height() / 2.);
    robot->setVisible(false);

    QPen pen = QPen(QColor(255, 0, 0));
    pen.setWidth(4);
    for(int i = 0; i < 4; i++) {
        echos[i] = new QGraphicsPathItem();
        echos[i]->setPen(pen);
        echos[i]->setPos(robot->transformOriginPoint());
        scene->addItem(echos[i]);
    }

    ui->plateau->setScene(scene);
    ////////////////////////////////////


    /* Préparation du réseau. */
    SocketCAN = new QTcpSocket(this);
    SocketIA = new QTcpSocket(this);

    connect(SocketCAN, SIGNAL(readyRead()), this, SLOT(ReadCAN()));
    connect(SocketIA,  SIGNAL(readyRead()), this, SLOT(ReadIA()));

    int portCAN, portIA;
    if(robotName == "petit") {
        portCAN = 7773;
        portIA  = 7774;
    }
    else {
        portCAN = 7777;
        portIA  = 7778;
    }
    SocketCAN->connectToHost(robotName, portCAN);
    SocketIA->connectToHost(robotName,  portIA);

    SocketCAN->waitForConnected();
    SocketIA->waitForConnected();
    ////////////////////////////////////



    chronoTimer = new QTimer(this);
    connect(chronoTimer, SIGNAL(timeout()), this, SLOT(RefreshChrono()));
    batteryTimer = new QTimer(this);
    connect(batteryTimer, SIGNAL(timeout()), this, SLOT(FileBattery()));
    batteryTimer->start(30000);
    FileBattery(); // Actualise le niveau batterie.

    WriteIA("MESSAGE minigui started on " + host.toUtf8() + " for " + robotName.toUtf8());
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


void MainWindow::RefreshRobot(int x, int y, int theta)
{
    robot->setVisible(true);
    robot->setRotation(180 - theta / 100.);
    robot->setPos(origin + QPointF(-x, y) * scale - robot->transformOriginPoint());
}


void MainWindow::RefreshEchos(QList<QByteArray> tokens)
{
    if(robot->isVisible() == false)
    {
        return;
    }

    for(int i = 0, j = 2; i < 3; i++) {
        if(j + 1 < tokens.count()) {
            qreal dist  = tokens.at(j++).toInt() * 10 * scale;
            qreal angle = tokens.at(j++).toInt();
            qreal span = 360. * 80. /* rayon apparent */ * scale / dist / 2 / 3.14159265f;
            QRectF rect = QRectF(robot->x() - dist, robot->y() - dist, dist * 2, dist * 2);

            QPainterPath path = QPainterPath();
            path.arcMoveTo(rect, angle - span / 2);
            path.arcTo(rect, angle - span / 2, span);

            echos[i]->setPath(path);

            echos[i]->setVisible(true);
        }
        else {
            echos[i]->setVisible(false);
        }
    }
}

void MainWindow::ReadCAN()
{
    if(!SocketCAN->canReadLine())
        return; // Not a full line yet.

    MainWindow::ParseCAN(SocketCAN->readLine().simplified());
}

void MainWindow::WriteCAN(QByteArray line)
{
    SocketCAN->write(line + "\n");
    ParseCAN(line); // Echo back to refresh view.
}

void MainWindow::ParseCAN(QByteArray line)
{
    ui->CANBrowser->append(line);
    line = line.toUpper();
    QList<QByteArray> tokens = line.split(' ');

    if(tokens.size() == 5 && tokens.at(0) == "ODO" && (tokens.at(1) == "POS" || tokens.at(1) == "SET")) {
        // Actualisation du robot.
        RefreshRobot(tokens.at(2).toInt(), tokens.at(3).toInt(), tokens.at(4).toInt());
    }
    else if(tokens.size() > 2 && tokens.at(0) == "TURRET" && tokens.at(1) == "ANSWER") {
        // Actualisation du radar.
        RefreshEchos(tokens);
    }
    else if(tokens.size() == 2 && tokens.at(0) == "TURRET" && tokens.at(1) == "OFF") {
        /*
        // On efface les echos.
        RefreshEchos(tokens);

        En fait non, de toute façon on reçoit des échos après.
        TODO: utiliser un timer pour dégager les vieux echos. */
    }
    else if(tokens.size() == 3 && tokens.at(0) == "BATTERY" && tokens.at(1) ==  "ANSWER") {
        QByteArray voltage = tokens.at(2);
        voltage.append(" V");
        ui->actionBattery->setText(voltage);
    }
}




void MainWindow::ReadIA()
{
    if(!SocketIA->canReadLine())
        return;

    MainWindow::ParseIA(SocketIA->readLine().simplified());
}

void MainWindow::WriteIA(QByteArray line)
{
    SocketIA->write(line + "\n");
    ParseIA(line); // Echo back to refresh view.
}

void MainWindow::ParseIA(QByteArray line)
{
    ui->IABrowser->append(line);
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
    pid_t pid = fork();
    WriteIA("MESSAGE minigui restart");
    if(pid == 0) {
        //sleep(1); // Le QWS se ferme.
        //execv(MainWindow::Argv[0], MainWindow::Argv);
        MainWindow::Argv[1] = NULL;
        execv(MainWindow::Argv[0], MainWindow::Argv);
    }
    else {
        //QApplication::exit(); // TODO
        hide();
    }
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
        qDebug() << system("killall python");

        pid_t pid = fork();
        if(pid == 0) {
            execlp("/home/ia/ia.py", "/home/ia/ia.py", robotName.toLatin1().data(), NULL);
            QApplication::exit(0);
        }

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





void MainWindow::AsservRotPos()
{
    WriteCAN("ASSERV ROT 36000");
}

void MainWindow::AsservRotNeg()
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
