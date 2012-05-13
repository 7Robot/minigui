#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QMessageBox>

#include <unistd.h> // Pour les execs sur ARM.



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
        qDebug() << system("killall python"); // TODO plus propre

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
