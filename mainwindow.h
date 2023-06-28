#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "machine.h"
#include "qcustomplot.h"
#include <QMainWindow>
#include <QTimer>

enum OnOff { On, Off };

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  // Lifecycle
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  // Attributes
  Ui::MainWindow *ui;
  Machine _machine;
  bool _running = false;
  QTimer *_bangBangTimer = new QTimer(this);
  QCPCurve *_xyCurveRapid;
  QCPCurve *_xyCurveInterp;
  QCPCurve *_xyCurvePosition;


  // Events
  void dragEnterEvent(QDragEnterEvent *e);
  void dropEvent(QDropEvent *);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void syncData();
  void toggleFormConections(enum OnOff);
  void setupMachineAfterNewINI();

  // Slots
private slots:
  void on_action_Open_INI_file_triggered();
  void on_machineDataChanged();
  void on_startButtonClicked();
  void on_formDataChanged();
  void on_outOfLimits(QString const &name);
  void on_bangBangTimeChanged(double arg1);
};
#endif // MAINWINDOW_H
