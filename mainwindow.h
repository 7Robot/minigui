#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void TestMoteurs();
    void TestOdometrie();
    void TestCapteurs();
    void FileQuit();
    void FileReboot();
    void FileHalt();

    void ResetPICs();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
