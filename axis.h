#ifndef AXIS_H
#define AXIS_H

#include <QObject>
#include <unistd.h>
#include <QThread>


class Axis : public QThread
{
    Q_OBJECT
public:
    explicit Axis(QObject *parent = nullptr, QString name = "_");
    ~Axis();
    QString inspect();
    void run() override;

signals:

public:
    double length = 1;
    double friction = 1000;
    double mass = 150;
    double max_torque = 20;
    double pitch = 0.01;
    double gravity = 0;
    useconds_t integration_dt = 5;
    double p = 1, i = 0, d = 0;
    int count = 0;

private:
    double _position = 0;
    double _setpoint = 0;
    double _speed = 0;
    double _time = 0;
    double _torque = 0;

};

#endif // AXIS_H
