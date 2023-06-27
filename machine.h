#ifndef MACHINE_H
#define MACHINE_H

#include "QtCore/qdebug.h"
#include "axis.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QHash>
#include <QObject>

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
  double link_axes(QList<char const *>);
  void describe();

  // Getters
  QString *brokerAddress() { return &_brokerAddress; }
  qint16 brokerPort() { return _brokerPort; }
  QString *pubTopic() { return &_pubTopic; }
  QString *subTopic() { return &_subTopic; }
  Axis *operator[](char const *name) {
    if (!_axesNames.contains(name)) {
      qDebug() << "Wrong axis " + QString(name);
      return nullptr;
    }
    return _axes[name];
  };

  // Attributes
public:
  QElapsedTimer *timer;
private:
  QString _brokerAddress;
  qint16 _brokerPort;
  QString _pubTopic;
  QString _subTopic;
  QList<char const *> _axesNames;
  QHash<char const *, Axis *> _axes;

  // Signals
signals:
  void dataHasChanged();
};

QDebug operator<<(QDebug dbg, Machine &m);

#endif // MACHINE_H
