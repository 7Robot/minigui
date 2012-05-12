#include "ui_mainwindow.h"
#include "mainwindow.h"
#include <unistd.h>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent, Qt::FramelessWindowHint),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    Vue(1);

    SocketCAN = new QTcpSocket(this);
    SocketIA = new QTcpSocket(this);

    connect(SocketCAN, SIGNAL(readyRead()), this, SLOT(ReadCAN()));
    connect(SocketIA,  SIGNAL(readyRead()), this, SLOT(ReadIA()));

    SocketCAN->connectToHost("localhost", 7773);
    SocketIA->connectToHost("localhost", 7774);

    SocketCAN->waitForConnected();
    SocketIA->waitForConnected();

    WriteIA("MESSAGE minigui started");
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::RefreshRobot(int x, int y, int theta)
{
    qDebug() << x << " " << y << " " << theta;
    // TODO
}



void MainWindow::WriteCAN(QByteArray line)
{
    SocketCAN->write(line);
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
    QList<QByteArray> tokens = line.toUpper().split(' ');
    ui->CANBrowser->append(line);

    if(tokens.size() == 5 && (line.startsWith("ODO POS") || line.startsWith("ODO SET"))) {
        RefreshRobot(tokens.at(2).toInt(), tokens.at(3).toInt(), tokens.at(4).toInt());
    }
    else if(line.startsWith("TURRET ANSWER")) {

        // TODO: effacer les echos précédents ici.

        for(int i = 2; i < tokens.count(); ) {
            int dist = tokens.at(i++).toInt();
            int angle = tokens.at(i++).toInt();
            qDebug() << dist << " " << angle;
            // TODO
        }
    }
    else if(tokens.size() == 3 && line.startsWith("BATTERY ANSWER")) {
        QByteArray voltage = tokens.at(2);
        voltage.append(" V");
        ui->actionBattery->setText(voltage);
    }
    else {
        // TODO textbox espion
        qDebug() << "Unrecognized: " << line << endl;
    }

    // FIXME: dessiner asserv speed ?
}



void MainWindow::WriteIA(QByteArray line)
{
    SocketIA->write(line);
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
    //QList<QByteArray> tokens = line.toUpper().split(' ');
    ui->IABrowser->append(line);
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
    msgBox.setText("Redémarrer l'ARM ?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if(msgBox.exec() == QMessageBox::Ok) {
        WriteIA("MESSAGE minigui reboot");
        qDebug() << system("reboot");
    }
}

void MainWindow::FileHalt()
{
    QMessageBox msgBox;
    msgBox.setText("Éteindre l'ARM ?");
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
        QApplication::exit();
    }
}

void MainWindow::FileQuit()
{
    WriteIA("MESSAGE minigui quit");
    QApplication::exit();
}



void MainWindow::OdoRouge()
{
    WriteCAN("ODO SET -750 1250 27000");
}

void MainWindow::OdoViolet()
{
    WriteCAN("ODO SET -750 -1250 9000");
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
    WriteIA("CALIBRATE 3");
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
