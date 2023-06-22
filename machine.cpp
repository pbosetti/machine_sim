#include "machine.h"
#include "toml.hpp"
#include <QDebug>

QDebug operator<<(QDebug dbg, Machine *m)
{
    QDebugStateSaver saver(dbg);
    dbg.nospace() << "Broker: " << *m->brokerAddress() + ":" + QString::number(m->brokerPort()) << "\n";
    dbg << "Publish on " << *m->pubTopic() << " - Subscribe to " << *m->subTopic() << "\n";
    return dbg;
}

Machine::Machine(QObject *parent)
    : QObject(parent),
    _brokerAddress(QString("localhost")),
    _brokerPort(1883),
    _pubTopic(QString("#")),
    _subTopic(QString("#")),
    _x(Axis(this, "X")),
    _y(Axis(this, "Y")),
    _z(Axis(this, "Z"))
{
//    setParent(parent);
}

Machine::~Machine() {}

void Machine::loadIniFile(QString &path) {
    auto config = toml::parse_file(path.toStdString());
    _brokerAddress = config["MQTT"]["broker_address"].value_or("localhost");
    _brokerPort = config["MQTT"]["broker_port"].value_or(1883);
    _subTopic = config["MQTT"]["pub_topic"].value_or("c-cnc/#");
    _pubTopic = config["MQTT"]["sub_topic"].value_or("c-cnc/#");
    // X
    _x.length = config["X"]["length"].value_or(1.0);
    _x.friction = config["X"]["friction"].value_or(1000.0);
    _x.mass = config["X"]["mass"].value_or(150.0);
    _x.max_torque = config["X"]["max_torque"].value_or(20.0);
    _x.pitch = config["X"]["pitch"].value_or(0.01);
    _x.gravity = config["X"]["gravity"].value_or(0.0);
    _x.integration_dt = config["X"]["integration_dt"].value_or(5);
    _x.p = config["X"]["p"].value_or(1.0);
    _x.i = config["X"]["i"].value_or(0.0);
    _x.d = config["X"]["d"].value_or(0.0);
    // Y
    _y.length = config["Y"]["length"].value_or(1.0);
    _y.friction = config["Y"]["friction"].value_or(1000.0);
    _y.mass = config["Y"]["mass"].value_or(150.0);
    _y.max_torque = config["Y"]["max_torque"].value_or(20.0);
    _y.pitch = config["Y"]["pitch"].value_or(0.01);
    _y.gravity = config["Y"]["gravity"].value_or(0.0);
    _y.integration_dt = config["Y"]["integration_dt"].value_or(5);
    _y.p = config["Y"]["p"].value_or(1.0);
    _y.i = config["Y"]["i"].value_or(0.0);
    _y.d = config["Y"]["d"].value_or(0.0);
    // Z
    _z.length = config["X"]["length"].value_or(1.0);
    _z.friction = config["Z"]["friction"].value_or(1000.0);
    _z.mass = config["Z"]["mass"].value_or(150.0);
    _z.max_torque = config["Z"]["max_torque"].value_or(20.0);
    _z.pitch = config["Z"]["pitch"].value_or(0.01);
    _z.gravity = config["Z"]["gravity"].value_or(0.0);
    _z.integration_dt = config["Z"]["integration_dt"].value_or(5);
    _z.p = config["Z"]["p"].value_or(1.0);
    _z.i = config["Z"]["i"].value_or(0.0);
    _z.d = config["Z"]["d"].value_or(0.0);
    qDebug() << this;
    emit dataHasChanged();
}

void Machine::start() {
    _x.start();
    _y.start();
    _z.start();
}

void Machine::stop() {
    _x.requestInterruption();
    _y.requestInterruption();
    _z.requestInterruption();
}


