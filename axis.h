#ifndef AXIS_H
#define AXIS_H

#include <QObject>
#include <QThread>
#include <unistd.h>

class Axis : public QThread {
  Q_OBJECT
public:
  // Lifecycle
  explicit Axis(QObject *parent = nullptr, QString name = "_");
  ~Axis();
  // Operations
  void run() override;
  void reset();

signals:
  void outOfLimits(QString const &axis);

  // Attributes
public:
  double length = 1;
  double friction = 1000;
  double mass = 150;
  double effective_mass = mass;
  double max_torque = 20;
  double pitch = 0.01;
  double gravity = 0;
  useconds_t integration_dt = 5;
  double p = 0, i = 0, d = 0;
  double count = 0;
  double setpoint = 0.0;

  // Getters
  double position() { return _position; }

  // Attributes
private:
  double _position = 0;
  double _speed = 0;
  double _time = 0;
  double _torque = 0;
  QElapsedTimer *_timer;
};

#endif // AXIS_H
