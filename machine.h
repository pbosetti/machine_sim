#ifndef MACHINE_H
#define MACHINE_H

#include "QtCore/qdebug.h"
#include "axis.h"
#include <QObject>
#include <QDebug>
#include <QHash>
#include <QElapsedTimer>

class Machine : public QObject
{
    Q_OBJECT
public:
    explicit Machine(QObject *parent = nullptr);
    ~Machine();
    void extracted(QList<QString> &list);
    void loadIniFile(QString &path);
    void start();
    void stop();
    void reset();


    QString *brokerAddress() { return &_brokerAddress; }
    qint16 brokerPort() { return _brokerPort; }
    QString *pubTopic() { return &_pubTopic; }
    QString *subTopic() { return &_subTopic; }
    Axis *axis(char const *name) { return _axes[name]; }

    QElapsedTimer *timer;

signals:
    void dataHasChanged();

private:
    QString _brokerAddress;
    qint16 _brokerPort;
    QString _pubTopic;
    QString _subTopic;
    QList<char const *> _axesNames;
    QHash<char const *, Axis *> _axes;

};

QDebug operator<<(QDebug dbg, Machine &m);


#endif // MACHINE_H
