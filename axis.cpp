#include "axis.h"
#include "machine.h"
#include <QDebug>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

Axis::Axis(QObject *parent, QString name) : QThread(parent) {
  setObjectName(name);
  qDebug() << "Created axis " << this->objectName();
  _timer = (dynamic_cast<Machine *>(parent))->timer;
  if (!_timer)
    qDebug() << "*** Casting error";
}

Axis::~Axis() {
  requestInterruption();
  wait();
}

void Axis::run() {
  qDebug() << "Starting thread for axis " << objectName()
           << " at time " + QString::number(_timer->nsecsElapsed());
  while (!isInterruptionRequested()) {
    count += setpoint;
    if (count > length * 1000 || count < 0) {
      emit outOfLimits(objectName());
      break;
    }
    sleep(1);
  }
  qDebug() << "Stopped thread for axis " << objectName()
           << " at time " + QString::number(_timer->nsecsElapsed());
}

void Axis::reset() {
  count = 0;
  setpoint = 0;
  _position = 0;
}
