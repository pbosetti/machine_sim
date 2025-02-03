#include "machine.h"
#include "toml.hpp"
#include <QDebug>
#include <QList>
#include <algorithm>
#include <math.h>
// #include <yaml/yaml.h>
#include <fstream>
#include <fkYAML/node.hpp>


QDebug operator<<(QDebug dbg, Machine *m) {
  QDebugStateSaver saver(dbg);
  dbg.nospace().noquote() << "Broker: "
                << *m->brokerAddress() + ":" + QString::number(m->brokerPort())
                << "\n";
  dbg << " Publish on " << *m->pubTopic() << " - Subscribe to " << *m->subTopic()
      << "\n";
  dbg << " Effective masses: X = "
      << QString::number((*m)[AxisTag::X]->effective_mass)
      << ", Y = " << QString::number((*m)[AxisTag::Y]->effective_mass)
      << ", Z = " << QString::number((*m)[AxisTag::Z]->effective_mass) << "\n";
  dbg << " Setpoint: X = "
      << QString::number((*m)[AxisTag::X]->setpoint())
      << ", Y = " << QString::number((*m)[AxisTag::Y]->setpoint())
      << ", Z = " << QString::number((*m)[AxisTag::Z]->setpoint()) << "\n";
  dbg << " Position: X = "
      << QString::number((*m)[AxisTag::X]->position())
      << ", Y = " << QString::number((*m)[AxisTag::Y]->position())
      << ", Z = " << QString::number((*m)[AxisTag::Z]->position()) << "\n";
  return dbg;
}


//   _     _  __                      _      
//  | |   (_)/ _| ___  ___ _   _  ___| | ___ 
//  | |   | | |_ / _ \/ __| | | |/ __| |/ _ \
//  | |___| |  _|  __/ (__| |_| | (__| |  __/
//  |_____|_|_|  \___|\___|\__, |\___|_|\___|
//                         |___/

Machine::Machine(QObject *parent)
    : QObject(parent), _brokerAddress(QString("localhost")), _brokerPort(1883),
      _pubTopic(QString("#")), _subTopic(QString("#")) {
  _axesTags = QList<AxisTag>({AxisTag::X, AxisTag::Y, AxisTag::Z});
  _axesNames = QHash<AxisTag, char const *>(
      {{AxisTag::X, "X"}, {AxisTag::Y, "Y"}, {AxisTag::Z, "Z"}});
  timer = new QElapsedTimer();
  _axes[AxisTag::X] = new Axis(this, "X");
  _axes[AxisTag::Y] = new Axis(this, "Y");
  _axes[AxisTag::Z] = new Axis(this, "Z");
}

Machine::~Machine() { stop(); }

//    ___                       _   _                 
//   / _ \ _ __   ___ _ __ __ _| |_(_) ___  _ __  ___ 
//  | | | | '_ \ / _ \ '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| | |_) |  __/ | | (_| | |_| | (_) | | | \__ \
//   \___/| .__/ \___|_|  \__,_|\__|_|\___/|_| |_|___/
//        |_|                                         

void Machine::loadIniFile(QString &path) {
  if (path.endsWith(".ini")) {
    auto config = toml::parse_file(path.toStdString());
    _brokerAddress = config["MQTT"]["broker_address"].value_or("localhost");
    _brokerPort = config["MQTT"]["broker_port"].value_or(1883);
    _subTopic = config["MQTT"]["pub_topic"].value_or("ccnc/setpoint");
    _pubTopic = config["MQTT"]["sub_topic"].value_or("ccnc/status#");
    _tq = config["C-CNC"]["tq"].value_or(0.005);

    for (auto &axis : _axesTags) {
      auto ax = _axesNames[axis];
      _axes[axis]->length = config[ax]["length"].value_or(1.0);
      _axes[axis]->friction = config[ax]["friction"].value_or(1000.0);
      _axes[axis]->mass = config[ax]["mass"].value_or(150.0);
      _axes[axis]->effective_mass = _axes[axis]->mass;
      _axes[axis]->max_torque = config[ax]["max_torque"].value_or(20.0);
      _axes[axis]->pitch = config[ax]["pitch"].value_or(0.01);
      _axes[axis]->gravity = config[ax]["gravity"].value_or(0.0);
      _axes[axis]->integration_dt =
          config[ax]["integration_dt"].value_or(5);
      _axes[axis]->p = config[ax]["p"].value_or(1.0);
      _axes[axis]->i = config[ax]["i"].value_or(0.0);
      _axes[axis]->d = config[ax]["d"].value_or(0.0);
    }
  } else if (path.endsWith(".yml")) {
      std::ifstream ifs(path.toStdString());
      auto config = fkyaml::node::deserialize(ifs);
      _brokerAddress = QString::fromStdString(config["mqtt"]["host"].get_value<std::string>());
    // auto config = YAML::LoadFile(path.toStdString());
    // _brokerAddress = QString::fromStdString(config["mqtt"]["host"].as<std::string>("localhost"));
    // _brokerPort = config["mqtt"]["port"].as<int>(1883);
    // _subTopic = QString::fromStdString(config["mqtt"]["topics"]["pub"].as<std::string>("cnc/setpoint"));
    // _pubTopic = QString::fromStdString(config["mqtt"]["topics"]["sub"].as<std::string>("cnc/status/#"));
    // _tq = config["machine"]["tq"].as<double>(0.005);

    // for (auto &axis: _axesTags) {
    //   auto ax = _axesNames[axis];
    //   _axes[axis]->length = config["machine"]["axes"][ax]["length"].as<double>(1.0);
    //   _axes[axis]->friction = config["machine"]["axes"][ax]["friction"].as<double>(1000.0);
    //   _axes[axis]->mass = config["machine"]["axes"][ax]["mass"].as<double>(150.0);
    //   _axes[axis]->effective_mass = _axes[axis]->mass;
    //   _axes[axis]->max_torque = config["machine"]["axes"][ax]["max_torque"].as<double>(20.0);
    //   _axes[axis]->pitch = config["machine"]["axes"][ax]["pitch"].as<double>(0.01);
    //   _axes[axis]->gravity = config["machine"]["axes"][ax]["gravity"].as<double>(0.0);
    //   _axes[axis]->integration_dt =
    //       config["machine"]["axes"][ax]["integration_dt"].as<double>(5);
    //   _axes[axis]->p = config["machine"]["axes"][ax]["p"].as<double>(1.0);
    //   _axes[axis]->i = config["machine"]["axes"][ax]["i"].as<double>(0.0);
    //   _axes[axis]->d = config["machine"]["axes"][ax]["d"].as<double>(0.0);
    // }
  } else {
    return;
  }
  link_axes({AxisTag::Z, AxisTag::Y, AxisTag::X});
  emit dataHasChanged();
}

void Machine::start() {
  for (AxisTag axis : _axesTags) {
    _axes[axis]->start();
  }
}

void Machine::stop() {
  for (AxisTag axis : _axesTags) {
    _axes[axis]->requestInterruption();
  }
}

void Machine::reset() {
  for (AxisTag axis : _axesTags) {
    _axes[axis]->reset();
  }
//  emit dataHasChanged();
}

double Machine::link_axes(QList<AxisTag> names) {
  double mass = 0;
  for (AxisTag axis : names) {
    mass += _axes[axis]->mass;
    _axes[axis]->effective_mass = mass;
  }
  return mass;
}

void Machine::describe() {
  qDebug().nospace() << "Machine:\n" << this;
}

quint64 Machine::lastTime() {
  QList<quint64> times = {
    _axes[AxisTag::X]->previousTime(),
    _axes[AxisTag::Y]->previousTime(),
    _axes[AxisTag::Z]->previousTime(),
  };
  QList<quint64>::iterator max = std::max_element(times.begin(), times.end());
  return *max;
}

double Machine::maxLength() {
  return std::max_element(
             _axes.begin(), _axes.end(),
             [](Axis *a, Axis *b) { return a->length < b->length; })
      .value()
      ->length;
}

double Machine::error() {
  return sqrt(
      pow(_axes[AxisTag::X]->position() - _axes[AxisTag::X]->setpoint(), 2) +
      pow(_axes[AxisTag::Y]->position() - _axes[AxisTag::Y]->setpoint(), 2) +
      pow(_axes[AxisTag::Z]->position() - _axes[AxisTag::Z]->setpoint(), 2));
}
