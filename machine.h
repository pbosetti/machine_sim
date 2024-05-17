#ifndef MACHINE_H
#define MACHINE_H

#include "QtCore/qdebug.h"
#include "axis.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QSettings>

//typedef enum {X, Y, Z} axisTag;
enum class AxisTag {X, Y, Z};

class Machine : public QObject {
  Q_OBJECT

public:
  // Lifecycle
  explicit Machine(QObject *parent = nullptr);
  ~Machine();

  // Operations
  void extracted(QList<QString> &list);
  void loadIniFile(QString &path);
  void start();
  void stop();
  void reset();
  double link_axes(QList<AxisTag>);
  void describe();
  quint64 lastTime();
  double maxLength();
  double error();
  void save_settings(QSettings &settings) {
    for (auto const &a: std::as_const(_axes)) {
      a->save_settings(settings);
    }
    settings.beginGroup("machine");
    settings.setValue("brokerAddress", _brokerAddress);
    settings.setValue("brokerPort", _brokerPort);
    settings.setValue("pubTopic", _pubTopic);
    settings.setValue("subTopic", _subTopic);
    settings.endGroup();
  }
  void read_settings(QSettings &settings) {
    for (auto const &a: std::as_const(_axes)) {
      a->read_settings(settings);
    }
    settings.beginGroup("machine");
    auto val = settings.value("brokerAddress");
    if (!val.isNull()) _brokerAddress = val.toString();
    val = settings.value("brokerPort");
    if (!val.isNull()) _brokerPort = val.toInt();
    val = settings.value("pubTopic");
    if (!val.isNull()) _pubTopic = val.toString();
    val = settings.value("subTopic");
    if (!val.isNull()) _subTopic = val.toString();
    settings.endGroup();
    link_axes({AxisTag::Z, AxisTag::Y, AxisTag::X});
    emit dataHasChanged();
  }

  // Getters
  QString *brokerAddress() { return &_brokerAddress; }
  qint16 brokerPort() { return _brokerPort; }
  QString *pubTopic() { return &_pubTopic; }
  QString *subTopic() { return &_subTopic; }
  Axis *operator[](AxisTag name) {
    if (!_axesNames.contains(name)) {
      qDebug() << "Wrong axis " + QString(_axesNames[name]);
      return nullptr;
    }
    return _axes[name];
  };
  QList<Axis *> axes() { return _axes.values(); }
  QList<AxisTag> *axesTags() { return &_axesTags; }
  QHash<AxisTag, char const *> *axesNames() { return &_axesNames; }
  double tq() { return _tq; }
  QList<QString> param_names() { return _axes[AxisTag::X]->param_names(); }

  // Setters
  void setBrokerAddress(QString addr) { _brokerAddress = addr; }
  void setBrokerPort(quint16 port) { _brokerPort = port; }
  void setPubTopic(QString s) { _pubTopic = s; }
  void setSubTopic(QString s) { _subTopic = s; }

  // Attributes
public:
  QElapsedTimer *timer;
private:
  QString _brokerAddress;
  qint16 _brokerPort;
  QString _pubTopic;
  QString _subTopic;
  QList<AxisTag> _axesTags;
  QHash<AxisTag, char const *> _axesNames;
  QHash<AxisTag, Axis *> _axes;
  double _tq = 0.005;

  // Signals
signals:
  void dataHasChanged();
};

QDebug operator<<(QDebug dbg, Machine *m);

#endif // MACHINE_H
