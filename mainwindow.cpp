#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent, Qt::FramelessWindowHint),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::TestCapteurs()
{
    QMessageBox msgBox;
    msgBox.setText("Test capteurs.");
    msgBox.exec();
}

void MainWindow::TestMoteurs()
{
    QMessageBox msgBox;
    msgBox.setText("Test moteurs.");
    msgBox.exec();
}

void MainWindow::TestOdometrie()
{
    QMessageBox msgBox;
    msgBox.setText("Test odométrie.");
}

void MainWindow::FileQuit()
{
    QApplication::exit();
}

void MainWindow::FileHalt()
{
    QMessageBox msgBox;
    msgBox.setText("Éteindre l'ARM ?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if(msgBox.exec() == QMessageBox::Ok)
        system("halt");
}

void MainWindow::FileReboot()
{
    QMessageBox msgBox;
    msgBox.setText("Redémarrer l'ARM ?");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    if(msgBox.exec() == QMessageBox::Ok)
        system("reboot");
}

void  MainWindow::ResetPICs() {

}
