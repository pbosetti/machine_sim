#include "machine.h"
#include "toml.hpp"
#include <QDebug>
#include <QList>
#include <algorithm>

QDebug operator<<(QDebug dbg, Machine *m) {
  QDebugStateSaver saver(dbg);
  dbg.nospace() << "Broker: "
                << *m->brokerAddress() + ":" + QString::number(m->brokerPort())
                << "\n";
  dbg << "Publish on " << *m->pubTopic() << " - Subscribe to " << *m->subTopic()
      << "\n";
  dbg << "Effective masses: X = "
      << QString::number(m->axis("X")->effective_mass)
      << ", Y = " << QString::number(m->axis("Y")->effective_mass)
      << ", Z = " << QString::number(m->axis("Z")->effective_mass) << "\n";
  return dbg;
}

Machine::Machine(QObject *parent)
    : QObject(parent), _brokerAddress(QString("localhost")), _brokerPort(1883),
      _pubTopic(QString("#")), _subTopic(QString("#")) {
  _axesNames = QList<char const *>({"X", "Y", "Z"});
  timer = new QElapsedTimer();
  _axes["X"] = new Axis(this, "X");
  _axes["Y"] = new Axis(this, "Y");
  _axes["Z"] = new Axis(this, "Z");
}

Machine::~Machine() { stop(); }

void Machine::loadIniFile(QString &path) {
  auto config = toml::parse_file(path.toStdString());
  _brokerAddress = config["MQTT"]["broker_address"].value_or("localhost");
  _brokerPort = config["MQTT"]["broker_port"].value_or(1883);
  _subTopic = config["MQTT"]["pub_topic"].value_or("c-cnc/#");
  _pubTopic = config["MQTT"]["sub_topic"].value_or("c-cnc/#");

  for (char const *axisName : _axesNames) {
    _axes[axisName]->length = config[axisName]["length"].value_or(1.0);
    _axes[axisName]->friction = config[axisName]["friction"].value_or(1000.0);
    _axes[axisName]->mass = config[axisName]["mass"].value_or(150.0);
    _axes[axisName]->max_torque = config[axisName]["max_torque"].value_or(20.0);
    _axes[axisName]->pitch = config[axisName]["pitch"].value_or(0.01);
    _axes[axisName]->gravity = config[axisName]["gravity"].value_or(0.0);
    _axes[axisName]->integration_dt =
        config[axisName]["integration_dt"].value_or(5);
    _axes[axisName]->p = config[axisName]["p"].value_or(1.0);
    _axes[axisName]->i = config[axisName]["i"].value_or(0.0);
    _axes[axisName]->d = config[axisName]["d"].value_or(0.0);
  }
  link_axes({"Z", "Y", "X"});
  emit dataHasChanged();
}

void Machine::start() {
  for (char const *axis : _axesNames) {
    _axes[axis]->start();
  }
}

void Machine::stop() {
  for (char const *axis : _axesNames) {
    _axes[axis]->requestInterruption();
  }
}

void Machine::reset() {
  for (char const *axis : _axesNames) {
    _axes[axis]->reset();
  }
  emit dataHasChanged();
}

double Machine::link_axes(QList<char const *> names) {
  double mass = 0;
  for (char const *axis : names) {
    mass += _axes[axis]->mass;
    _axes[axis]->effective_mass = mass;
  }
  return mass;
}
