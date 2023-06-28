#include "mainwindow.h"
#include "machine.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>

#define SETPOINT_SLIDER_MAX 100.0f

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

  // Enable signals from GUI form to machine
  toggleFormConections(On);

  // disable start button on load (requires loading a INI file first)
  ui->startButton->setEnabled(false);
  ui->startButton->setToolTip("Load an INI file first");

  ui->setPointXSlider->setMaximum(SETPOINT_SLIDER_MAX);
  ui->setPointYSlider->setMaximum(SETPOINT_SLIDER_MAX);
  ui->setPointZSlider->setMaximum(SETPOINT_SLIDER_MAX);

  // Setup timePlot
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
  ui->timePlot->yAxis->setRange(0, 1);
  ui->timePlot->setInteraction(QCP::iRangeDrag, true);
  ui->timePlot->axisRect()->setRangeDrag(Qt::Horizontal);
  ui->timePlot->setInteraction(QCP::iRangeZoom, true);
  ui->timePlot->axisRect()->setRangeZoom(Qt::Horizontal);
  ui->timePlot->xAxis->setLabel(QString("Elapsed time"));
  ui->timePlot->yAxis->setLabel(QString("Axis Position (mm)"));

  // Timed action for reading data from axes
  QTimer *timerPlot = new QTimer(this);
  connect(timerPlot, &QTimer::timeout, this, [this]() {
    if (!_running)
      return;
    double x = _machine[AxisTag::X]->position();
    double y = _machine[AxisTag::Y]->position();
    double z = _machine[AxisTag::Z]->position();
    double t = _machine.lastTime() / 1.0E9;
    ui->xPositionBar->setValue(x * 1000.0);
    ui->yPositionBar->setValue(y * 1000.0);
    ui->zPositionBar->setValue(z * 1000.0);
    ui->timePlot->graph(3)->addData(t, x);
    ui->timePlot->graph(4)->addData(t, y);
    ui->timePlot->graph(5)->addData(t, z);
    ui->timePlot->xAxis->setRange(t, 60, Qt::AlignRight);
    ui->timePlot->replot();
  });
  timerPlot->start(20);

  connect(_bangBangTimer, &QTimer::timeout, this, [this]() {
    static int level = +1;
    if (ui->bangBangX->isChecked()) {
      _machine[AxisTag::X]->setpoint =
          (0.5 + level * ui->bangBangLevel->value()) *
          _machine[AxisTag::X]->length;
    }
    if (ui->bangBangY->isChecked()) {
      _machine[AxisTag::Y]->setpoint =
          (0.5 + level * ui->bangBangLevel->value()) *
          _machine[AxisTag::Y]->length;
    }
    if (ui->bangBangZ->isChecked()) {
      _machine[AxisTag::Z]->setpoint =
          (0.5 + level * ui->bangBangLevel->value()) *
          _machine[AxisTag::Z]->length;
    }
    level *= -1;
  });
  _bangBangTimer->start(ui->bangBangTime->value()*1000);
}

MainWindow::~MainWindow() { delete ui; }

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
  syncData();
}

void MainWindow::syncData() {
  toggleFormConections(Off);
  ui->xPositionBar->setMaximum(_machine[AxisTag::X]->length * 1000);
  ui->yPositionBar->setMaximum(_machine[AxisTag::Y]->length * 1000);
  ui->zPositionBar->setMaximum(_machine[AxisTag::Z]->length * 1000);

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

  _machine[AxisTag::X]->setpoint = ui->setPointXSlider->value() /
                                   SETPOINT_SLIDER_MAX *
                                   _machine[AxisTag::X]->length;
  _machine[AxisTag::Y]->setpoint = ui->setPointYSlider->value() /
                                   SETPOINT_SLIDER_MAX *
                                   _machine[AxisTag::Y]->length;
  _machine[AxisTag::Z]->setpoint = ui->setPointZSlider->value() /
                                   SETPOINT_SLIDER_MAX *
                                   _machine[AxisTag::Z]->length;
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

// One of the exes reaches its limit
void MainWindow::on_outOfLimits(QString const &name) {
  _running = false;
  _machine.stop();
  _machine.reset();
  ui->startButton->setText("Start");
  ui->statusbar->showMessage(
      "Limits reached on axis " + name + ", stopping execution", 10000);
}

void MainWindow::on_pushButton_clicked() {
  _machine.describe();
  //  ui->xPositionBar->setValue()
}

void MainWindow::on_bangBangTime_valueChanged(double arg1) {
  _bangBangTimer->setInterval(arg1 * 1000);
}
