#include "mainwindow.h"
#include "machine.h"
#include "ui_mainwindow.h"
#include "machine_params.h"
#include "doubleclickslider.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QSettings>
#include <stdio.h>

#define SETPOINT_SLIDER_MAX 100.0f
#define BUFLEN 1024

#define PBAR_STYLE "QProgressBar {border: 1px solid grey; border-radius: 0px;text-align: center;}\n"
#define PBAR_STYLE_OK PBAR_STYLE "QProgressBar::chunk {background: blue}"
#define PBAR_STYLE_SAT PBAR_STYLE "QProgressBar::chunk {background: red}"

//   _     _  __                      _
//  | |   (_)/ _| ___  ___ _   _  ___| | ___
//  | |   | | |_ / _ \/ __| | | |/ __| |/ _ \
//  | |___| |  _|  __/ (__| |_| | (__| |  __/
//  |_____|_|_|  \___|\___|\__, |\___|_|\___|
//                         |___/

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setLocale(QLocale("EN_en"));
  setAcceptDrops(true);

  auto val = _settings.value("initialized");
  if (!val.isNull()) {
    _machine.read_settings(_settings);
    ui->xpSpinBox->setValue(_machine[AxisTag::X]->p);
    ui->xiSpinBox->setValue(_machine[AxisTag::X]->i);
    ui->xdSpinBox->setValue(_machine[AxisTag::X]->d);

    ui->ypSpinBox->setValue(_machine[AxisTag::Y]->p);
    ui->yiSpinBox->setValue(_machine[AxisTag::Y]->i);
    ui->ydSpinBox->setValue(_machine[AxisTag::Y]->d);

    ui->zpSpinBox->setValue(_machine[AxisTag::Z]->p);
    ui->ziSpinBox->setValue(_machine[AxisTag::Z]->i);
    ui->zdSpinBox->setValue(_machine[AxisTag::Z]->d);
    setupMachineAfterNewINI();
    on_machineDataChanged();
  } else {
    // disable start button on load (requires loading a INI file first)
    ui->startButton->setEnabled(false);
    ui->startButton->setToolTip("Load an INI file first");
  }

  timeZoomH.setCheckable(true);
  timeZoomV.setCheckable(true);
  timeZoomBoth.setCheckable(true);
  timePlotZoomGroup.addAction(&timeZoomH);
  timePlotZoomGroup.addAction(&timeZoomV);
  timePlotZoomGroup.addAction(&timeZoomBoth);
  traceZoomH.setCheckable(true);
  traceZoomV.setCheckable(true);
  traceZoomBoth.setCheckable(true);
  tracePlotZoomGroup.addAction(&traceZoomH);
  tracePlotZoomGroup.addAction(&traceZoomV);
  tracePlotZoomGroup.addAction(&traceZoomBoth);
  ui->menuZoom->addSeparator()->setText("Time plot");
  ui->menuZoom->addAction(&timeZoomBoth);
  ui->menuZoom->addAction(&timeZoomH);
  ui->menuZoom->addAction(&timeZoomV);
  ui->menuZoom->addSeparator()->setText("Trace plot");
  ui->menuZoom->addSeparator();
  ui->menuZoom->addAction(&traceZoomBoth);
  ui->menuZoom->addAction(&traceZoomH);
  ui->menuZoom->addAction(&traceZoomV);
  timeZoomBoth.setChecked(true);
  traceZoomBoth.setChecked(true);

  ui->xTorqueBar->setStyleSheet(PBAR_STYLE_OK);
  ui->yTorqueBar->setStyleSheet(PBAR_STYLE_OK);
  ui->zTorqueBar->setStyleSheet(PBAR_STYLE_OK);

  connect(&timePlotZoomGroup, SIGNAL(triggered(QAction*)), this,
          SLOT(on_actionTimeZoom(QAction*)));
  connect(&tracePlotZoomGroup, SIGNAL(triggered(QAction*)), this,
           SLOT(on_actionTimeZoom(QAction*)));

  // Set up MQTT
  _client = new QMqttClient(this);
  connect(_client, SIGNAL(disconnected()), this, SLOT(on_brokerDisconnected()));
  connect(_client, &QMqttClient::messageReceived, this,
          &MainWindow::on_mqttMessage);
  connect(_client, SIGNAL(connected()), this, SLOT(on_brokerConnected()));

  // Out of limits signal
  for (AxisTag axis : *_machine.axesTags()) {
    connect(_machine[axis], SIGNAL(outOfLimits(QString)), this,
            SLOT(on_outOfLimits(QString)));
  }

  // when INI file loaded, update GUI
  connect(&_machine, SIGNAL(dataHasChanged()), this,
          SLOT(on_machineDataChanged()));

  // GUI
  connect(ui->startButton, SIGNAL(clicked(bool)), this,
          SLOT(on_startButtonClicked()));

  connect(ui->bangBangTime, SIGNAL(valueChanged(double)), this,
          SLOT(on_bangBangTimeChanged(double)));

  connect(ui->connectButton, SIGNAL(clicked(bool)), this,
          SLOT(on_connectButtonClicked()));

  connect(ui->setPointXSlider, &DoubleClickSlider::sliderHandleDoubleClicked, this, [=]() {
    ui->setPointXSlider->setValue(SETPOINT_SLIDER_MAX / 2.0);
  });

  connect(ui->parametersButton, SIGNAL(clicked(bool)), this, SLOT(on_actionSetParameters_triggered()));

  connect(ui->resetButton, &QPushButton::clicked, this, [=]() {_machine.reset();});

  // Enable signals from GUI form to machine
  toggleFormConections(On);


  ui->setPointXSlider->setMaximum(SETPOINT_SLIDER_MAX);
  ui->setPointXSlider->setTickInterval(SETPOINT_SLIDER_MAX / 4.0);
  ui->setPointYSlider->setMaximum(SETPOINT_SLIDER_MAX);
  ui->setPointYSlider->setTickInterval(SETPOINT_SLIDER_MAX / 4.0);
  ui->setPointZSlider->setMaximum(SETPOINT_SLIDER_MAX);
  ui->setPointZSlider->setTickInterval(SETPOINT_SLIDER_MAX / 4.0);

  // Setup timePlot
  ui->timePlot->setLocale(QLocale("EN_en"));
  QPen pen;
  pen.setColor(QColor(200, 0, 0));
  pen.setStyle(Qt::CustomDashLine);
  QList<qreal> dashes;
  dashes << 20 << 2;
  pen.setDashPattern(dashes);
  // X
  ui->timePlot->addGraph();
  ui->timePlot->graph(0)->setPen(pen);

  // Y
  pen.setColor(QColor(0, 200, 0));
  ui->timePlot->addGraph();
  ui->timePlot->graph(1)->setPen(pen);

  // Z
  pen.setColor(QColor(0, 0, 200));
  ui->timePlot->addGraph();
  ui->timePlot->graph(2)->setPen(pen);

  pen.setStyle(Qt::SolidLine);
  // X
  pen.setColor(QColor(255, 0, 0));
  ui->timePlot->addGraph();
  ui->timePlot->graph(3)->setPen(pen);

  // Y
  pen.setColor(QColor(0, 255, 0));
  ui->timePlot->addGraph();
  ui->timePlot->graph(4)->setPen(pen);

  // Z
  pen.setColor(QColor(0, 0, 255));
  ui->timePlot->addGraph();
  ui->timePlot->graph(5)->setPen(pen);

  QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
  timeTicker->setTimeFormat("%h:%m:%s");
  ui->timePlot->xAxis->setTicker(timeTicker);
  ui->timePlot->axisRect()->setupFullAxesBox();
  ui->timePlot->yAxis->setRange(0, _machine.maxLength());
  ui->timePlot->setInteraction(QCP::iRangeDrag, true);
  ui->timePlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
  ui->timePlot->setInteraction(QCP::iRangeZoom, true);
  ui->timePlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
  ui->timePlot->xAxis->setLabel(QString("Elapsed time"));
  ui->timePlot->yAxis->setLabel(QString("Axis Position (m)"));
  // double click reset zoom
  connect(ui->timePlot, &QCustomPlot::mouseDoubleClick, this, [=](){
    ui->timePlot->yAxis->setRange(0, _machine[AxisTag::Y]->length);
      ui->timePlot->replot();
  });
//  connect(this, SIGNA


  // Setup XY plot
  _xyCurveRapid = new QCPCurve(ui->tracePlot->xAxis, ui->tracePlot->yAxis);
  _xyCurveRapid->setPen(QPen(QColor(200, 0, 0)));
  _xyCurveInterp = new QCPCurve(ui->tracePlot->xAxis, ui->tracePlot->yAxis);
  _xyCurveInterp->setPen(QPen(QColor(0, 0, 200)));
  _xyCurvePosition = new QCPCurve(ui->tracePlot->xAxis, ui->tracePlot->yAxis);
  _xyCurvePosition->setPen(QPen(QColor(0, 200, 0)));
  ui->tracePlot->setLocale(QLocale("EN_en"));
  ui->tracePlot->axisRect()->setupFullAxesBox();
  ui->tracePlot->xAxis->setRange(0, _machine[AxisTag::X]->length);
  ui->tracePlot->yAxis->setRange(0, _machine[AxisTag::Y]->length);
  ui->tracePlot->setInteraction(QCP::iRangeDrag, true);
  ui->tracePlot->axisRect()->setRangeDrag(Qt::Horizontal | Qt::Vertical);
  ui->tracePlot->setInteraction(QCP::iRangeZoom, true);
  ui->tracePlot->xAxis->setLabel(QString("X (m)"));
  ui->tracePlot->yAxis->setLabel(QString("Y (m)"));
  // Double click reset zoom
  connect(ui->tracePlot, &QCustomPlot::mouseDoubleClick, this, [=](){
    ui->tracePlot->xAxis->setRange(0, _machine[AxisTag::X]->length);
    ui->tracePlot->yAxis->setRange(0, _machine[AxisTag::Y]->length);
    ui->tracePlot->replot();
  });

  // Timed action for reading data from axes
  QTimer *plotTimer = new QTimer(this);
  connect(plotTimer, &QTimer::timeout, this, [this]() {
    if (ui->actionKeepAspectRatio->isChecked()) {
      ui->tracePlot->yAxis->setScaleRatio(ui->tracePlot->xAxis, 1);
    }
    if (!_running)
      return;
    double x = _machine[AxisTag::X]->position();
    double y = _machine[AxisTag::Y]->position();
    double z = _machine[AxisTag::Z]->position();
    double px = _machine[AxisTag::X]->torque();
    double py = _machine[AxisTag::Y]->torque();
    double pz = _machine[AxisTag::Z]->torque();
    double t = _machine.lastTime() / 1.0E9;
    ui->xPositionText->display(x * 1000.0);
    ui->yPositionText->display(y * 1000.0);
    ui->zPositionText->display(z * 1000.0);
    ui->xTorqueBar->setValue(fabs(px) * 100);
    ui->yTorqueBar->setValue(fabs(py) * 100);
    ui->zTorqueBar->setValue(fabs(pz) * 100);
    ui->timePlot->graph(3)->addData(t, x);
    ui->timePlot->graph(4)->addData(t, y);
    ui->timePlot->graph(5)->addData(t, z);
    ui->timePlot->xAxis->setRange(t, 60, Qt::AlignRight);
    ui->timePlot->replot();
    _xyCurvePosition->addData(x, y);
    ui->tracePlot->replot();
    ui->xTorqueBar->setStyleSheet(_machine[AxisTag::X]->saturate() ? PBAR_STYLE_SAT : PBAR_STYLE_OK);
    ui->yTorqueBar->setStyleSheet(_machine[AxisTag::Y]->saturate() ? PBAR_STYLE_SAT : PBAR_STYLE_OK);
    ui->zTorqueBar->setStyleSheet(_machine[AxisTag::Z]->saturate() ? PBAR_STYLE_SAT : PBAR_STYLE_OK);
  });
  plotTimer->start(20);

  connect(_bangBangTimer, &QTimer::timeout, this, [this]() {
    static int level = +1;
    if (ui->bangBangX->isChecked()) {
      _machine[AxisTag::X]->setpoint(
          (0.5 + level * ui->bangBangLevel->value()) *
          _machine[AxisTag::X]->length);
    }
    if (ui->bangBangY->isChecked()) {
      _machine[AxisTag::Y]->setpoint(
          (0.5 + level * ui->bangBangLevel->value()) *
          _machine[AxisTag::Y]->length);
    }
    if (ui->bangBangZ->isChecked()) {
      _machine[AxisTag::Z]->setpoint(
          (0.5 + level * ui->bangBangLevel->value()) *
          _machine[AxisTag::Z]->length);
    }
    level *= -1;
  });
  _bangBangTimer->start(ui->bangBangTime->value()*1000);

  connect(_publishTimer, &QTimer::timeout, this, [this]() {
    static char buffer[BUFLEN];
    if (_rapid) {
      snprintf(buffer, BUFLEN, "%f", _machine.error());
      _client->publish(_errorTopic, QByteArray(buffer, strlen(buffer)));
      snprintf(buffer, BUFLEN, "%f,%f,%f", _machine[AxisTag::X]->position(), _machine[AxisTag::Y]->position(), _machine[AxisTag::Z]->position());
      _client->publish(_positionTopic, QByteArray(buffer, strlen(buffer)));
    }
  });

  // connect(_machine[AxisTag::X], &Axis::saturate, this, [=](QString name, bool sat) {
  //   if (name == "X") {
  //     ui->xTorqueBar->setStyleSheet(sat ? PBAR_STYLE_SAT : PBAR_STYLE_OK);
  //   } else if (name == "Y") {
  //     ui->yTorqueBar->setStyleSheet(sat ? PBAR_STYLE_SAT : PBAR_STYLE_OK);
  //   } else if (name == "Z") {
  //     ui->zTorqueBar->setStyleSheet(sat ? PBAR_STYLE_SAT : PBAR_STYLE_OK);
  //   }
  // });

//  QTimer *debugTimer = new QTimer(this);
//  connect(debugTimer, &QTimer::timeout, this, [=]() {
//    if (!_running) return;
//    double e =  _machine[AxisTag::Z]->setpoint - _machine[AxisTag::Z]->position();
//    double ei = _machine[AxisTag::Z]->_errI;
//    double ed = _machine[AxisTag::Z]->_errD;
//    double s = _machine[AxisTag::Z]->_speed;
//    double t =  _machine[AxisTag::Z]->_torque;
//    qDebug() << "e: " << e <<
//        ", ei: " << ei <<
//        ", ed: " << ed <<
//        ", T: " << t <<
//        ", v: " << s;
//  });
//  debugTimer->start(250);
}

MainWindow::~MainWindow() {
  _client->disconnectFromHost();
  _machine.save_settings(_settings);
  _settings.setValue("initialized", true);
  delete ui;
}

//    ___                       _   _
//   / _ \ _ __   ___ _ __ __ _| |_(_) ___  _ __  ___
//  | | | | '_ \ / _ \ '__/ _` | __| |/ _ \| '_ \/ __|
//  | |_| | |_) |  __/ | | (_| | |_| | (_) | | | \__ \
//   \___/| .__/ \___|_|  \__,_|\__|_|\___/|_| |_|___/
//        |_|

// Update GUI when a new INI file is loaded
void MainWindow::on_machineDataChanged() {
  ui->brokerAddressField->setText(*_machine.brokerAddress());
  ui->brokerPortField->setValue(_machine.brokerPort());
  ui->pubTopicField->setText(*_machine.pubTopic());
  ui->subTopicField->setText(*_machine.subTopic());
  QString publishBasePath = *_machine.pubTopic();
  publishBasePath.removeLast();
  _errorTopic = publishBasePath + "error";
  _positionTopic = publishBasePath + "position";
  qDebug() << "publish to " << _errorTopic << "and" << _positionTopic;
  ui->timePlot->yAxis->setRange(0, _machine.maxLength());
  ui->timePlot->replot();
  ui->tracePlot->xAxis->setRange(0, _machine[AxisTag::X]->length);
  ui->tracePlot->yAxis->setRange(0, _machine[AxisTag::Y]->length);
  ui->tracePlot->replot();
  syncData();
}

void MainWindow::syncData() {
  toggleFormConections(Off);
  ui->xTorqueBar->setMaximum(_machine[AxisTag::X]->max_torque * 100);
  ui->yTorqueBar->setMaximum(_machine[AxisTag::Y]->max_torque * 100);
  ui->zTorqueBar->setMaximum(_machine[AxisTag::Z]->max_torque * 100);

  ui->xpSpinBox->setValue(_machine[AxisTag::X]->p);
  ui->xiSpinBox->setValue(_machine[AxisTag::X]->i);
  ui->xdSpinBox->setValue(_machine[AxisTag::X]->d);

  ui->ypSpinBox->setValue(_machine[AxisTag::Y]->p);
  ui->yiSpinBox->setValue(_machine[AxisTag::Y]->i);
  ui->ydSpinBox->setValue(_machine[AxisTag::Y]->d);

  ui->zpSpinBox->setValue(_machine[AxisTag::Z]->p);
  ui->ziSpinBox->setValue(_machine[AxisTag::Z]->i);
  ui->zdSpinBox->setValue(_machine[AxisTag::Z]->d);

  ui->setPointXSlider->setValue(SETPOINT_SLIDER_MAX / 2.0);
  ui->setPointYSlider->setValue(SETPOINT_SLIDER_MAX / 2.0);
  ui->setPointZSlider->setValue(SETPOINT_SLIDER_MAX / 2.0);
  toggleFormConections(On);
}

// Enables/disables connections: if not, there's a backlash on INI drag'n'drop
void MainWindow::toggleFormConections(enum OnOff state) {
  if (On == state) {
    connect(ui->xpSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(on_formDataChanged()));
    connect(ui->xiSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(on_formDataChanged()));
    connect(ui->xdSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(on_formDataChanged()));

    connect(ui->ypSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(on_formDataChanged()));
    connect(ui->yiSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(on_formDataChanged()));
    connect(ui->ydSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(on_formDataChanged()));

    connect(ui->zpSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(on_formDataChanged()));
    connect(ui->ziSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(on_formDataChanged()));
    connect(ui->zdSpinBox, SIGNAL(valueChanged(double)), this,
            SLOT(on_formDataChanged()));

    connect(ui->setPointXSlider, SIGNAL(valueChanged(int)), this,
            SLOT(on_formDataChanged()));
    connect(ui->setPointYSlider, SIGNAL(valueChanged(int)), this,
            SLOT(on_formDataChanged()));
    connect(ui->setPointZSlider, SIGNAL(valueChanged(int)), this,
            SLOT(on_formDataChanged()));
  } else {
    disconnect(ui->xpSpinBox, nullptr, nullptr, nullptr);
    disconnect(ui->xiSpinBox, nullptr, nullptr, nullptr);
    disconnect(ui->xdSpinBox, nullptr, nullptr, nullptr);

    disconnect(ui->ypSpinBox, nullptr, nullptr, nullptr);
    disconnect(ui->yiSpinBox, nullptr, nullptr, nullptr);
    disconnect(ui->ydSpinBox, nullptr, nullptr, nullptr);

    disconnect(ui->zpSpinBox, nullptr, nullptr, nullptr);
    disconnect(ui->ziSpinBox, nullptr, nullptr, nullptr);
    disconnect(ui->zdSpinBox, nullptr, nullptr, nullptr);
  }
}

void MainWindow::setupMachineAfterNewINI() {
  ui->setPointXSlider->setValue(SETPOINT_SLIDER_MAX / 2.0);
  ui->setPointYSlider->setValue(SETPOINT_SLIDER_MAX / 2.0);
  ui->setPointZSlider->setValue(SETPOINT_SLIDER_MAX / 2.0);
  ui->startButton->setEnabled(true);
  ui->startButton->setToolTip("Start simulation");
  ui->connectButton->setEnabled(true);
}

//   _____                 _
//  | ____|_   _____ _ __ | |_ ___
//  |  _| \ \ / / _ \ '_ \| __/ __|
//  | |___ \ V /  __/ | | | |_\__ \
//  |_____| \_/ \___|_| |_|\__|___/

// Accept dragged INI file
void MainWindow::dragEnterEvent(QDragEnterEvent *e) {
  if (e->mimeData()->hasUrls()) {
    QList urls = e->mimeData()->urls();
    if (urls.count() != 1) {
      statusBar()->showMessage("One file only, please!");
    } else {
      QUrl url = urls.first();
      if (url.path().endsWith(".ini")) {
        statusBar()->showMessage(
            QString("Release to open ") + url.toLocalFile(), 10000);
        e->acceptProposedAction();
      } else {
        statusBar()->showMessage("Wrong file type");
      }
    }
  }
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent *) {
  statusBar()->clearMessage();
}

// Load dragged INI file into machine
void MainWindow::dropEvent(QDropEvent *e) {
  QUrl url = e->mimeData()->urls().first();
  QString fileName = url.toLocalFile();
  statusBar()->showMessage(QString("Opening ") + fileName);
  _machine.loadIniFile(fileName);
  setupMachineAfterNewINI();
}

//   ____  _       _
//  / ___|| | ___ | |_ ___
//  \___ \| |/ _ \| __/ __|
//   ___) | | (_) | |_\__ \
//  |____/|_|\___/ \__|___/

void MainWindow::on_actionTimeZoom(QAction *action) {
  if (action == &timeZoomBoth) {
    ui->timePlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
  } else if (action == &timeZoomH) {
    ui->timePlot->axisRect()->setRangeZoom(Qt::Horizontal);
  } else if (action == &timeZoomV) {
    ui->timePlot->axisRect()->setRangeZoom(Qt::Vertical);
  } else if (action == &traceZoomBoth) {
    ui->tracePlot->axisRect()->setRangeZoom(Qt::Horizontal | Qt::Vertical);
  } else if (action == &traceZoomH) {
    ui->tracePlot->axisRect()->setRangeZoom(Qt::Horizontal);
  } else if (action == &traceZoomV) {
    ui->tracePlot->axisRect()->setRangeZoom(Qt::Vertical);
  }
}

// Update machine when GUI data change
void MainWindow::on_formDataChanged() {
  _machine[AxisTag::X]->p = ui->xpSpinBox->value();
  _machine[AxisTag::X]->i = ui->xiSpinBox->value();
  _machine[AxisTag::X]->d = ui->xdSpinBox->value();

  _machine[AxisTag::Y]->p = ui->ypSpinBox->value();
  _machine[AxisTag::Y]->i = ui->yiSpinBox->value();
  _machine[AxisTag::Y]->d = ui->ydSpinBox->value();

  _machine[AxisTag::Z]->p = ui->zpSpinBox->value();
  _machine[AxisTag::Z]->i = ui->ziSpinBox->value();
  _machine[AxisTag::Z]->d = ui->zdSpinBox->value();

  _machine[AxisTag::X]->setpoint(ui->setPointXSlider->value() /
                                   SETPOINT_SLIDER_MAX *
                                  _machine[AxisTag::X]->length);
  _machine[AxisTag::Y]->setpoint(ui->setPointYSlider->value() /
                                   SETPOINT_SLIDER_MAX *
                                  _machine[AxisTag::Y]->length);
  _machine[AxisTag::Z]->setpoint(ui->setPointZSlider->value() /
                                  SETPOINT_SLIDER_MAX *
                                  _machine[AxisTag::Z]->length);
}

void MainWindow::on_action_Open_INI_file_triggered() {
  QString fileName =
      QFileDialog::getOpenFileName(this, "Open INI file", "", "");
  if (fileName.isNull())
    return;
  _machine.loadIniFile(fileName);
  setupMachineAfterNewINI();
}

// Start/stop simulation
void MainWindow::on_startButtonClicked() {
  if (!_running) {
    for (int i = 0; i < ui->timePlot->graphCount(); i++) {
      ui->timePlot->graph(i)->data().data()->clear();
      _xyCurvePosition->data().data()->clear();
      _xyCurveInterp->data().data()->clear();
      _xyCurveRapid->data().data()->clear();
    }
    _machine.start();
    _running = true;
    ui->startButton->setText("Stop");
    _machine.timer->start();
  } else {
    _machine.stop();
    _machine.reset();
    _running = false;
    ui->startButton->setText("Start");
  }
}

void MainWindow::on_connectButtonClicked() {
  bool disconnected = (_client->state() == QMqttClient::Disconnected);
  if (disconnected) {
    _client->setHostname(ui->brokerAddressField->text());
    _client->setPort(ui->brokerPortField->value());
    _client->connectToHost();
    _publishTimer->setInterval(_machine.tq() * 1000);
    _publishTimer->start();
  } else {
    _client->disconnectFromHost();
    _publishTimer->stop();
  }
  ui->brokerAddressField->setEnabled(!disconnected);
  ui->brokerPortField->setEnabled(!disconnected);
  ui->pubTopicField->setEnabled(!disconnected);
  ui->subTopicField->setEnabled(!disconnected);
  ui->connectButton->setText(disconnected ? "Disconnect" : "Connect");
  statusBar()->showMessage(disconnected ? "Connected" : "Disconnected", 2000);
}

void MainWindow::on_brokerConnected() {
  auto subscription = _client->subscribe(ui->subTopicField->text());
  if (!subscription) {
    QMessageBox::critical(this, "Error",
                          "Could not subscribe. Is there a valid connection?");
    return;
  } else {
    statusBar()->showMessage(
        QString("Subscribed to topic ") + ui->subTopicField->text(), 2000);
  }
}

void MainWindow::on_brokerDisconnected() {
  ui->brokerAddressField->setEnabled(true);
  ui->brokerPortField->setEnabled(true);
  ui->connectButton->setText(QString("Connect"));
  statusBar()->showMessage(QString("Unexpected disconnection"), 20000);
}

void MainWindow::on_mqttMessage(const QByteArray &message,
                                const QMqttTopicName &topic) {
  if (topic.name().endsWith(QString("setpoint"))) {
    double t = _machine.lastTime() / 1.0E9;
    QJsonDocument doc = QJsonDocument::fromJson(message);
    QJsonObject obj = doc.object();
    double x = obj["x"].toDouble() / 1000.0;
    double y = obj["y"].toDouble() / 1000.0;
    double z = obj["z"].toDouble() / 1000.0;
    int rapid = obj["rapid"].toInt();
    _rapid = (1 == rapid);
    _machine[AxisTag::X]->setpoint(x);
    _machine[AxisTag::Y]->setpoint(y);
    _machine[AxisTag::Z]->setpoint(z);
    ui->timePlot->graph(0)->addData(t, x);
    ui->timePlot->graph(1)->addData(t, y);
    ui->timePlot->graph(2)->addData(t, z);
    ui->timePlot->xAxis->setRange(t, 60, Qt::AlignRight);
    (rapid ? _xyCurveRapid : _xyCurveInterp)->addData(x, y);
  } else if (topic.name().endsWith(QString("position"))) {

  }
}

// One of the exes reaches its limit
void MainWindow::on_outOfLimits(QString const &name) {
  _running = false;
  _machine.stop();
  _machine.reset();
  ui->startButton->setText("Start");
  ui->statusbar->showMessage(
      "Limits reached on axis " + name + ", stopping execution", 10000);
}


void MainWindow::on_bangBangTimeChanged(double arg1) {
  _bangBangTimer->setInterval(arg1 * 1000);
}


void MainWindow::on_actionSetParameters_triggered() {
  MachineParamsDialog dialog(this);
  dialog.setModal(true);
  dialog.setSizeGripEnabled(false);
  QMap<QString, AxisTag> axesMap = {{"x", AxisTag::X}, {"y", AxisTag::Y}, {"z", AxisTag::Z}};

  if (dialog.exec() == QDialog::Accepted) {
    QMessageBox confirm(this);
    confirm.setText("Axes timesteps are different: are you sure?");
    confirm.setStandardButtons(QMessageBox::Yes);
    confirm.addButton(QMessageBox::No);
    confirm.setDefaultButton(QMessageBox::No);
    if (dialog.valueByName("xTimestep") != dialog.valueByName("yTimestep") || dialog.valueByName("yTimestep") != dialog.valueByName("zTimestep")) {
      if (confirm.exec() == QMessageBox::No) { goto unchanged; }
    }
    foreach (auto &axis, axesMap.keys()) {
      for (auto const &p: _machine.param_names()) {
        auto pc = p;
        pc.front() = pc.front().toUpper();
        *(*_machine[axesMap[axis]])[p] = dialog.valueByName(axis, pc);
      }
    }
  } else {
    goto unchanged;
  }
  return;
  unchanged:
  ui->statusbar->showMessage("Machine parameters unchanged", 5000);
}
