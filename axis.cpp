#include "axis.h"
#include "machine.h"
#include <QDebug>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

//   _     _  __                      _      
//  | |   (_)/ _| ___  ___ _   _  ___| | ___ 
//  | |   | | |_ / _ \/ __| | | |/ __| |/ _ \
//  | |___| |  _|  __/ (__| |_| | (__| |  __/
//  |_____|_|_|  \___|\___|\__, |\___|_|\___|
//                         |___/             

Axis::Axis(QObject *parent, QString name) : QThread(parent) {
  setObjectName(name);
  _timer = (dynamic_cast<Machine *>(parent))->timer;
  if (!_timer)
    qDebug() << "*** Casting error";
}

Axis::~Axis() {
  requestInterruption();
  wait();
}

//    ___                       _   _                 
//   / _ \ _ __   ___ _ __ __ _| |_(_) ___  _ __  ___ 
//  | | | | '_ \ / _ \ '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| | |_) |  __/ | | (_| | |_| | (_) | | | \__ \
//   \___/| .__/ \___|_|  \__,_|\__|_|\___/|_| |_|___/
//        |_|                                         

void Axis::run() {
  double dt = 0;
  double m = effective_mass;
  double f = 0;
  quint64 now = _timer->nsecsElapsed();
  _previousTime = now;
  qDebug() << "Starting thread for axis " << objectName()
           << " at time " + QString::number(now);
  while (!isInterruptionRequested()) {
    // timings
    now = _timer->nsecsElapsed();
    dt = (now - _previousTime) / 1.0E9;
    _previousTime = now;
    // PID
    pid(dt);
    // fwd dynamics (Euler's scheme)
    f = (M_PI * _torque / pitch) - (GRAVITY * mass * pitch);
    _position += _speed * dt;
    _speed = _speed * (1 - friction / m * dt) + f * dt / m;
    // out-of-range conditions
    if (_position > length || _position < 0) {
      emit outOfLimits(objectName());
      break;
    }
    QObject().thread()->usleep((unsigned long)integration_dt);
  }
  qDebug() << "Stopped thread for axis " << objectName()
           << " at time " + QString::number(_timer->nsecsElapsed());
}

void Axis::reset() {
  setpoint = length / 2.0;
  _position = length / 2.0;
  _speed = 0;
  _torque = 0;
  _time = 0;
  _previousTime = 0;
  _errI = _errD = 0;
  _prevError = 0;
}


void Axis::pid(double dt) {
  double out, err;
  err = setpoint - _position;
  if (i)
    _errI += err * dt * 0.9;
  if (d && dt > 0)
    _errD = (err - _prevError) / dt;
  _prevError = err;
  out = p * err + i * _errI + d * _errD;
  if (out > 0)
    _torque = fmin(out, max_torque);
  else
    _torque = fmax(out, -max_torque);
}
