#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "machine.h"
#include "qcustomplot.h"
#include <QMainWindow>
#include <QTimer>
#include <QMqttClient>
#include <QSettings>
#include <QDir>
#include <QtCharts/QtCharts>

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

  Machine * machine() { return &_machine; }

private:
  // Attributes
  Ui::MainWindow *ui;
  Machine _machine;
  bool _running = false;
  QTimer *_bangBangTimer = new QTimer(this);
  QTimer *_publishTimer = new QTimer(this);
  QCPCurve *_xyCurveRapid;
  QCPCurve *_xyCurveInterp;
  QCPCurve *_xyCurvePosition;
  QMqttClient *_client;
  QString *_publishBasePath = new QString();
  QMqttTopicName _errorTopic;
  QMqttTopicName _positionTopic;
  bool _rapid = false;
  QSettings _settings = QSettings("UniTN", "machine_sim");

  QActionGroup timePlotZoomGroup = QActionGroup(this);
  QAction timeZoomH = QAction("Time Zoom horizontal", this);
  QAction timeZoomV = QAction("Time Zoom vertical", this);
  QAction timeZoomBoth = QAction("Time Zoom both", this);
  QActionGroup tracePlotZoomGroup = QActionGroup(this);
  QAction traceZoomH = QAction("Trace Zoom horizontal", this);
  QAction traceZoomV = QAction("Trace Zoom vertical", this);
  QAction traceZoomBoth = QAction("Trace Zoom both", this);

  QString _logFileBaseName = "";
  uint16_t _logNumber = 0;
  QFile _logFile;
  QBarSet *_pidSetP = new QBarSet("P");
  QBarSet *_pidSetI = new QBarSet("I");
  QBarSet *_pidSetD = new QBarSet("D");
  QStackedBarSeries *_barSeries = new QStackedBarSeries;


  QString logFilename();

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
  void on_action_select_log_destination_triggered();
  void logButton_triggered(bool checked);
  void on_actionTimeZoom(QAction *action);
  void on_machineDataChanged();
  void on_startButtonClicked();
  void on_connectButtonClicked();
  void on_formDataChanged();
  void on_outOfLimits(QString const &name);
  void on_bangBangTimeChanged(double arg1);
  void on_brokerDisconnected();
  void on_brokerConnected();
  void on_mqttMessage(const QByteArray &message, const QMqttTopicName &topic);
  void on_actionSetParameters_triggered();
};
#endif // MAINWINDOW_H
