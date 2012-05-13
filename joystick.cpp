#include "joystick.h"
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>

Joystick::Joystick(QWidget *parent) :
    QLabel(parent)
{

}

void Joystick::mousePressEvent(QMouseEvent *event)
{
    qDebug() << event->x() << " " << event->y();
}

void Joystick::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << event->x() << " " << event->y();
}
