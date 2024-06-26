#ifndef AXIS_H
#define AXIS_H

#include "QtCore/qdebug.h"
#include <QObject>
#include <QThread>
#include <QSettings>
//#include <unistd.h>

#define GRAVITY 9.81f

class Axis : public QThread {
  Q_OBJECT
public:
  // Lifecycle
  explicit Axis(QObject *parent = nullptr, QString name = "_");
  ~Axis();
  // Operations
  void run() override;
  void reset();
  void save_settings(QSettings &settings);
  void read_settings(QSettings &settings);
  QString info();

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
  double integration_dt = 5;
  double p = 0, i = 0, d = 0;
  double rt_pacing = 1.0;

  // Getters
  double position() { return _position; }
  quint64 previousTime() { return _previousTime; }
  double * operator[](QString name) { return _params[name]; }
  QList<QString> param_names() { return _params.keys(); }
  double torque() { return _torque; }
  bool saturate() { return _saturate; }
  void setpoint(double val) { _setpoint = val; _errI = 0; }
  double setpoint() { return _setpoint; }
  QList<double> pidValues();

  // Attributes
private:
  QMap<QString, double*> _params = {
    {"length", &length},
    {"friction", &friction},
    {"mass", &mass},
    {"maxTorque", &max_torque},
    {"pitch", &pitch},
    {"gravity", &gravity},
    {"timestep", &integration_dt},
  };
  double _setpoint = 0.0;
  double _position = 0;
  double _time = 0;
  QElapsedTimer *_timer;
  quint64 _previousTime = 0;
  double _speed = 0;
  double _torque = 0;
  double _errI = 0, _errD = 0;
  double _prevError = 0;
  bool _saturate = false;

  // Utilities
private:
  void pid(double dt);
};

#endif // AXIS_H
