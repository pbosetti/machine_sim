#include "axis.h"
#include "machine.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>


Axis::Axis(QObject *parent, QString name)
    : QThread(parent)
{
    setObjectName(name);
    qDebug() << "Created axis " << this->objectName();
    _timer = (dynamic_cast<Machine *>(parent))->timer;
    if (!_timer) qDebug() << "*** Casting error";
}

Axis::~Axis() {
    requestInterruption();
    wait();
}

QString Axis::inspect() {
    QString result = QString("Axis " + objectName() + "\n");
    result += QString().asprintf("length: %f, friction: %f, mass: %f\n", length, friction, mass);
    result += QString().asprintf("max_torque: %f, pitch: %f, gravity: %f, dt: %d\n", max_torque, pitch, gravity, integration_dt);
    result += QString().asprintf("p: %f, i: %f, d: %f\n", p, i, d);
    return result;
}

void Axis::run() {
    qDebug() << "Starting thread for axis " << objectName() << " at time " + QString::number(_timer->nsecsElapsed());
    while (!isInterruptionRequested()) {
        count += setpoint;
        if (count > length * 1000 || count < 0) {
            emit outOfLimits(objectName());
            break;
        }
        sleep(1);
    }
    qDebug() << "Stopped thread for axis " << objectName() << " at time " + QString::number(_timer->nsecsElapsed());
}


void Axis::reset() {
    count = 0;
    setpoint = 0;
    _position = 0;
}
