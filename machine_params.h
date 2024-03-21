#ifndef MACHINE_PARAMS_H
#define MACHINE_PARAMS_H

#include "ui_machine_params.h"
#include "mainwindow.h"
#include "machine.h"
#include "axis.h"

class MachineParamsDialog : public QDialog {
  Q_OBJECT;

public:
  MachineParamsDialog(MainWindow *parent = nullptr) : QDialog(parent), ui(new Ui::Dialog) {
    ui->setupUi(this);
    _machine = parent->machine();
    setWindowTitle("Machine parameters");

    ui->xLength->setSuffix(" m");
    ui->xLength->setValue((*_machine)[AxisTag::X]->length);
    ui->xFriction->setValue((*_machine)[AxisTag::X]->friction);
    ui->xMass->setValue((*_machine)[AxisTag::X]->mass);
    ui->xMass->setSuffix(" kg");
    ui->xMaxTorque->setValue((*_machine)[AxisTag::X]->max_torque);
    ui->xMaxTorque->setSuffix(" N m");
    ui->xPitch->setValue((*_machine)[AxisTag::X]->pitch);
    ui->xPitch->setSuffix(" m/rev");
    ui->xGravity->setValue((*_machine)[AxisTag::X]->gravity);
    ui->xGravity->setSuffix(" m/s/s");
    ui->xTimestep->setValue((*_machine)[AxisTag::X]->integration_dt);
    ui->xTimestep->setSuffix(" ms");

    ui->yLength->setSuffix(" m");
    ui->yLength->setValue((*_machine)[AxisTag::Y]->length);
    ui->yFriction->setValue((*_machine)[AxisTag::Y]->friction);
    ui->yMass->setValue((*_machine)[AxisTag::Y]->mass);
    ui->yMass->setSuffix(" kg");
    ui->yMaxTorque->setValue((*_machine)[AxisTag::Y]->max_torque);
    ui->yMaxTorque->setSuffix(" N m");
    ui->yPitch->setValue((*_machine)[AxisTag::Y]->pitch);
    ui->yPitch->setSuffix(" m/rev");
    ui->yGravity->setValue((*_machine)[AxisTag::Y]->gravity);
    ui->yGravity->setSuffix(" m/s/s");
    ui->yTimestep->setValue((*_machine)[AxisTag::Y]->integration_dt);
    ui->yTimestep->setSuffix(" ms");

    ui->zLength->setSuffix(" m");
    ui->zLength->setValue((*_machine)[AxisTag::Z]->length);
    ui->zFriction->setValue((*_machine)[AxisTag::Z]->friction);
    ui->zMass->setValue((*_machine)[AxisTag::Z]->mass);
    ui->zMass->setSuffix(" kg");
    ui->zMaxTorque->setValue((*_machine)[AxisTag::Z]->max_torque);
    ui->zMaxTorque->setSuffix(" N m");
    ui->zPitch->setValue((*_machine)[AxisTag::Z]->pitch);
    ui->zPitch->setSuffix(" m/rev");
    ui->zGravity->setValue((*_machine)[AxisTag::Z]->gravity);
    ui->zGravity->setSuffix(" m/s/s");
    ui->zTimestep->setValue((*_machine)[AxisTag::Z]->integration_dt);
    ui->zTimestep->setSuffix(" ms");
  }

  ~MachineParamsDialog() {
  }

  double valueByName(QString axis, QString name = "") {
    auto widget = findChild<QDoubleSpinBox*>(axis + name);
    return widget->value();
  }

private:
  Ui::Dialog *ui;
  Machine *_machine;
};



#endif // MACHINE_PARAMS_H
