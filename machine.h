#ifndef MACHINE_H
#define MACHINE_H

#include "QtCore/qdebug.h"
#include "axis.h"
#include <QDebug>
#include <QElapsedTimer>
#include <QHash>
#include <QObject>

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
  QList<AxisTag> *axesTags() { return &_axesTags; }
  QHash<AxisTag, char const *> *axesNames() { return &_axesNames; }

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

  // Signals
signals:
  void dataHasChanged();
};

QDebug operator<<(QDebug dbg, Machine &m);

#endif // MACHINE_H
