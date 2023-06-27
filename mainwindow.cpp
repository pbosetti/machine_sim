#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setAcceptDrops(true);

  // Out of limits signal
  connect(machine.axis("X"), SIGNAL(outOfLimits(QString)), this,
          SLOT(on_outOfLimits(QString)));
  connect(machine.axis("Y"), SIGNAL(outOfLimits(QString)), this,
          SLOT(on_outOfLimits(QString)));
  connect(machine.axis("Z"), SIGNAL(outOfLimits(QString)), this,
          SLOT(on_outOfLimits(QString)));

  // when INI file loaded, update GUI
  connect(&machine, SIGNAL(dataHasChanged()), this,
          SLOT(on_machineDataChanged()));

  // Enable signals from GUI form to machine
  toggleFormConections(On);

  // Timed action for reading data from axes
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, [this]() {
    int x = machine.axis("X")->count;
    int y = machine.axis("Y")->count;
    int z = machine.axis("Z")->count;
    //        qDebug() << "X Count: " + QString::number(x) +
    //                    ", Y Count: " + QString::number(y) +
    //                    ", Z Count: " + QString::number(z);
    ui->xPositionBar->setValue(x);
    ui->yPositionBar->setValue(y);
    ui->zPositionBar->setValue(z);
  });
  timer->start(500);
}

MainWindow::~MainWindow() { delete ui; }

// Update GUI when a new INI file is loaded
void MainWindow::on_machineDataChanged() {
  ui->brokerAddressField->setText(*machine.brokerAddress());
  ui->brokerPortField->setValue(machine.brokerPort());
  ui->pubTopicField->setText(*machine.pubTopic());
  ui->subTopicField->setText(*machine.subTopic());
  syncData();
}

// Update machine when GUI data change
void MainWindow::on_formDataChanged() {
  machine.axis("X")->p = ui->xpSpinBox->value();
  machine.axis("X")->i = ui->xiSpinBox->value();
  machine.axis("X")->d = ui->xdSpinBox->value();

  machine.axis("Y")->p = ui->ypSpinBox->value();
  machine.axis("Y")->i = ui->yiSpinBox->value();
  machine.axis("Y")->d = ui->ydSpinBox->value();

  machine.axis("Z")->p = ui->zpSpinBox->value();
  machine.axis("Z")->i = ui->ziSpinBox->value();
  machine.axis("Z")->d = ui->zdSpinBox->value();

  machine.axis("X")->setpoint = ui->setPointXSlider->value();
  machine.axis("Y")->setpoint = ui->setPointYSlider->value();
  machine.axis("Z")->setpoint = ui->setPointZSlider->value();
}

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
  machine.loadIniFile(fileName);
}

void MainWindow::on_action_Open_INI_file_triggered() {
  QString fileName =
      QFileDialog::getOpenFileName(this, "Open INI file", "", "");
  if (fileName.isNull())
    return;
  machine.loadIniFile(fileName);
}

// Start/stop simulation
void MainWindow::on_startButton_clicked() {
  if (!_running) {
    machine.start();
    _running = true;
    ui->startButton->setText("Stop");
    machine.timer->start();
  } else {
    machine.stop();
    _running = false;
    ui->startButton->setText("Start");
  }
}

// One of the exes reaches its limit
void MainWindow::on_outOfLimits(QString const &name) {
  _running = false;
  machine.stop();
  machine.reset();
  ui->startButton->setText("Start");
  ui->statusbar->showMessage(
      "Limits reached on axis " + name + ", stopping execution", 10000);
}

void MainWindow::syncData() {
  toggleFormConections(Off);
  ui->setPointXSlider->setMaximum(machine.axis("X")->length * 1000);
  ui->setPointYSlider->setMaximum(machine.axis("Y")->length * 1000);
  ui->setPointZSlider->setMaximum(machine.axis("Z")->length * 1000);

  ui->xPositionBar->setMaximum(machine.axis("X")->length * 1000);
  ui->yPositionBar->setMaximum(machine.axis("Y")->length * 1000);
  ui->zPositionBar->setMaximum(machine.axis("Z")->length * 1000);

  ui->xpSpinBox->setValue(machine.axis("X")->p);
  ui->xiSpinBox->setValue(machine.axis("X")->i);
  ui->xdSpinBox->setValue(machine.axis("X")->d);

  ui->ypSpinBox->setValue(machine.axis("Y")->p);
  ui->yiSpinBox->setValue(machine.axis("Y")->i);
  ui->ydSpinBox->setValue(machine.axis("Y")->d);

  ui->zpSpinBox->setValue(machine.axis("Z")->p);
  ui->ziSpinBox->setValue(machine.axis("Z")->i);
  ui->zdSpinBox->setValue(machine.axis("Z")->d);
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
