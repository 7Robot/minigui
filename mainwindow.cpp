#include "ui_mainwindow.h"
#include "mainwindow.h"
#include <unistd.h>
#include <QDebug>
#include <QMessageBox>
#include <QtCore/qmath.h>
#include <QPen>
#include <QBrush>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QHostInfo>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent, Qt::FramelessWindowHint),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    chrono = new QTimer(this);
    connect(chrono, SIGNAL(timeout()), this, SLOT(RefreshChrono()));

    Vue(2);

    // Préparation de la vue Plateau. */
    scene = new QGraphicsScene(0, 0, 320, 213, this);
    background = scene->addPixmap(QPixmap(":/pics/plateau.png"));
    robot = scene->addPixmap(QPixmap(":/pics/robot.png"));
    robot->setTransformOriginPoint(21.5, 21.5);
    robot->setVisible(false);

    QPen pen = QPen(QColor(255, 0, 0));
    pen.setWidth(4);
    for(int i = 0; i < 4; i++) {
        echos[i] = new QGraphicsPathItem(robot);
        echos[i]->setPen(pen);
        echos[i]->setPos(robot->transformOriginPoint());
    }

    ui->plateau->setScene(scene);
    ////////////////////////////////////


    /* Préparation du réseau. */
    SocketCAN = new QTcpSocket(this);
    SocketIA = new QTcpSocket(this);

    connect(SocketCAN, SIGNAL(readyRead()), this, SLOT(ReadCAN()));
    connect(SocketIA,  SIGNAL(readyRead()), this, SLOT(ReadIA()));

    QString host = QHostInfo::localHostName();
    if(host != "gros")
        host = "petit";

    int portCAN, portIA;
    if(host == "petit") {
        portCAN = 7773;
        portIA  = 7774;
    }
    else {
        portCAN = 7777;
        portIA = 7778;
    }
    SocketCAN->connectToHost(host, portCAN);
    SocketIA->connectToHost(host,  portIA);

    SocketCAN->waitForConnected();
    SocketIA->waitForConnected();
    ////////////////////////////////////



    WriteIA("MESSAGE minigui started on " + host.toUtf8());
}

MainWindow::~MainWindow()
{
    delete ui;
}


QPointF MainWindow::origin(160, 106.5);
void MainWindow::RefreshRobot(int x, int y, int theta)
{
    robot->setVisible(true);
    robot->setRotation(theta / 100.);
    robot->setPos(origin + QPointF(-x, y) * scale - robot->transformOriginPoint());
}


void MainWindow::WriteCAN(QByteArray line)
{
    SocketCAN->write(line + "\n");
    ParseCAN(line); // Echo back to refresh view.
}

void MainWindow::ReadCAN()
{
    if(!SocketCAN->canReadLine())
        return; // Not a full line yet.

    MainWindow::ParseCAN(SocketCAN->readLine().trimmed());
}

void MainWindow::ParseCAN(QByteArray line)
{
    ui->CANBrowser->append(line);
    line = line.toUpper();
    QList<QByteArray> tokens = line.split(' ');

    if(tokens.size() == 5 && (line.startsWith("ODO POS") || line.startsWith("ODO SET"))) {
        RefreshRobot(tokens.at(2).toInt(), tokens.at(3).toInt(), tokens.at(4).toInt());
    }
    else if(line.startsWith("TURRET ANSWER")) {
        for(int i = 0, j = 2; i < 3; i++) {
            if(j + 1 < tokens.count()) {
                qreal dist  = tokens.at(j++).toInt() * 10 * scale;
                qreal angle = tokens.at(j++).toInt();

                qreal span = 360. * 80. /* rayon apparent */ * scale / dist / 2 / 3.14159265f;
                QRectF rect = QRectF(-dist, -dist, dist * 2, dist * 2);

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
    else if(tokens.size() == 3 && line.startsWith("BATTERY ANSWER")) {
        QByteArray voltage = tokens.at(2);
        voltage.append(" V");
        ui->actionBattery->setText(voltage);
    }
}



void MainWindow::WriteIA(QByteArray line)
{
    SocketIA->write(line + "\n");
    ParseIA(line); // Echo back to refresh view.
}

void MainWindow::ReadIA()
{
    if(!SocketIA->canReadLine())
        return;

    MainWindow::ParseIA(SocketIA->readLine().trimmed());
}

void MainWindow::ParseIA(QByteArray line)
{
    ui->IABrowser->append(line);
    line = line.toUpper();
    QList<QByteArray> tokens = line.split(' ');

    if(tokens.size() == 1 && line.startsWith("START")) {
        chrono->start(1000);
        matchStart.start();
        RefreshChrono();
    }
    else if(tokens.size() == 1 && line.startsWith("STOP")) {
        if(chrono->isActive())
            RefreshChrono();
        chrono->stop();
    }
}

void MainWindow::RefreshChrono()
{
    int remaining = (90 - matchStart.elapsed() / 1000);
    qDebug() << remaining;
    ui->chrono->display(remaining);
}



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

char ** MainWindow::Argv; // Les arguments de main() à peu de chose près.

void MainWindow::FileRestart()
{
    pid_t pid = fork();
    WriteIA("MESSAGE minigui restart");
    if(pid == 0) {
        execv(MainWindow::Argv[0], MainWindow::Argv); // TODO marche pas.
    }
    else {
        //TODO QApplication::exit();
        hide();
    }
}

void MainWindow::FileQuit()
{
    WriteIA("MESSAGE minigui quit");
    QApplication::exit();
}



void MainWindow::OdoRouge()
{
    WriteCAN("ODO SET 1250 750 9000");
}

void MainWindow::OdoViolet()
{
    WriteCAN("ODO SET -1250 750 27000");
}

void MainWindow::OdoRecalage()
{
    WriteIA("POSITIONING");
}

void MainWindow::OdoMute()
{
    WriteCAN("ODO MUTE");
}

void MainWindow::OdoUnmute()
{
    WriteCAN("ODO UNMUTE");
}

void MainWindow::OdoRequest()
{
    WriteCAN("ODO REQUEST");
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



void MainWindow::Vue(int vue)
{
    ui->messages->setVisible(vue == 1);
    ui->plateau ->setVisible(vue == 2);
    ui->match   ->setVisible(vue == 3);

    ui->actionMessages->setChecked(vue == 1);
    ui->actionPlateau ->setChecked(vue == 2);
    ui->actionMatch   ->setChecked(vue == 3);
}

void MainWindow::VueMessages()
{
    Vue(1);
}

void MainWindow::VuePlateau()
{
    Vue(2);
}

void MainWindow::VueMatch()
{
    Vue(3);
}
