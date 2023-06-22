#ifndef MACHINE_H
#define MACHINE_H

#include "QtCore/qdebug.h"
#include "axis.h"
#include <QObject>
#include <QDebug>
#include <QHash>


class Machine : public QObject
{
    Q_OBJECT
public:
    explicit Machine(QObject *parent = nullptr);
    ~Machine();
    void loadIniFile(QString &path);
    void start();
    void stop();


    QString *brokerAddress() { return &_brokerAddress; }
    qint16 brokerPort() { return _brokerPort; }
    QString *pubTopic() { return &_pubTopic; }
    QString *subTopic() { return &_subTopic; }
    Axis *x() { return &_x; }
    Axis *y() { return &_y; }
    Axis *z() { return &_z; }

signals:
    void dataHasChanged();

private:
    QString _brokerAddress;
    qint16 _brokerPort;
    QString _pubTopic;
    QString _subTopic;
    Axis _x, _y, _z;
    QHash<QString, Axis> _axes;

};

QDebug operator<<(QDebug dbg, Machine &m);


#endif // MACHINE_H
