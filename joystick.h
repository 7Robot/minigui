#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <QLabel>

class Joystick : public QLabel
{
    Q_OBJECT
public:
    explicit Joystick(QWidget *parent = 0);

protected:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent (QMouseEvent * event);

signals:

public slots:

};

#endif // JOYSTICK_H
