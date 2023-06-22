#include "axis.h"
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>


Axis::Axis(QObject *parent, QString name)
    : QThread(parent)
{
    setObjectName(name);
    qDebug() << "Created axis " << this->objectName();
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
    qDebug() << "Starting thread for axis " << objectName();
    while (!isInterruptionRequested()) {
        count += p;
        sleep(1);
    }
    qDebug() << "Stopped thread for axis " << objectName();
}
