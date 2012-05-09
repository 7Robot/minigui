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

    SocketCAN = new QTcpSocket(this);
    SocketIA = new QTcpSocket(this);

    connect(SocketCAN, SIGNAL(readyRead()), this, SLOT(ReadCAN()));
    connect(SocketIA,  SIGNAL(readyRead()), this, SLOT(ReadIA()));

    SocketCAN->connectToHost("localhost", 7773);
    SocketIA->connectToHost("localhost", 7774);

    SocketCAN->waitForConnected();
    SocketIA->waitForConnected();

    SocketIA->write("message minigui started\n");
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

void MainWindow::ReadCAN()
{
    if(!SocketCAN->canReadLine())
        return; // Not a full line yet.

    MainWindow::ParseCAN(SocketCAN->readLine().trimmed().toUpper());
}

void MainWindow::ParseCAN(QByteArray line)
{
    QList<QByteArray> tokens = line.split(' ');

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
        qDebug() << "unrecognized: " << line;
    }

    // FIXME: dessiner asserv speed ?
}

void MainWindow::WriteBackCAN(QByteArray line)
{
    SocketCAN->write(line);
    ParseCAN(line); // Echo back to refresh view.
}

void MainWindow::ReadIA()
{
    if(!SocketIA->canReadLine())
        return;

    QByteArray line = SocketIA->readLine();

    // TODO textbox espion erreurs ?

    //QMessageBox msgBox;
    //msgBox.setText("Unexpected : " + line);
    //msgBox.exec();
}


void  MainWindow::FileBattery()
{
    WriteBackCAN("BATTERY REQUEST\n");
}

void  MainWindow::FileReset()
{
    WriteBackCAN("RESET\n");
}

char ** MainWindow::Argv; // Les arguments de main() à peu de chose près.

void MainWindow::FileRestart()
{
    pid_t pid = fork();
    SocketIA->write("message minigui restart\n");
    if(pid == 0) {
        execv(MainWindow::Argv[0], MainWindow::Argv); // TODO
    }
    else {
        QApplication::exit();
    }
}

void MainWindow::FileQuit()
{
    SocketIA->write("message minigui quit\n");
    QApplication::exit();
}

void MainWindow::FileReboot(){
    QMessageBox msgBox;
    msgBox.setText("Redémarrer l'ARM ?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if(msgBox.exec() == QMessageBox::Ok) {
        SocketIA->write("message minigui reboot\n");
        qDebug() << system("reboot");
    }
}

void MainWindow::FileHalt()
{
    QMessageBox msgBox;
    msgBox.setText("Éteindre l'ARM ?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if(msgBox.exec() == QMessageBox::Ok) {
        SocketIA->write("message minigui halt\n");
        qDebug() << system("halt");
    }
}


void MainWindow::OdoMute()
{
    WriteBackCAN("ODO MUTE\n");
}

void MainWindow::OdoUnmute()
{
    WriteBackCAN("odo unmute\n"); // TODO CAsssssssse !
}

void MainWindow::OdoRouge()
{
    WriteBackCAN("odo set -750 1250 27000\n");
}

void MainWindow::OdoViolet()
{
    WriteBackCAN("odo set -750 -1250 9000\n");
}

void MainWindow::Cmd1()
{
    SocketIA->write("cmd1\n");
}

void MainWindow::Cmd2()
{
    SocketIA->write("cmd2\n");
}

void MainWindow::Cmd3()
{
    SocketIA->write("cmd3\n");
}

void MainWindow::Cmd4()
{
    SocketIA->write("cmd4\n");
}

void MainWindow::Cmd5()
{
    SocketIA->write("cmd5\n");
}
