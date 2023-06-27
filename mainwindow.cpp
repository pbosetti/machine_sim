#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QMimeData>
#include <QTimer>

//   _     _  __                      _      
//  | |   (_)/ _| ___  ___ _   _  ___| | ___ 
//  | |   | | |_ / _ \/ __| | | |/ __| |/ _ \
//  | |___| |  _|  __/ (__| |_| | (__| |  __/
//  |_____|_|_|  \___|\___|\__, |\___|_|\___|
//                         |___/             

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setAcceptDrops(true);

  // Out of limits signal
  connect(_machine["X"], SIGNAL(outOfLimits(QString)), this,
          SLOT(on_outOfLimits(QString)));
  connect(_machine["Y"], SIGNAL(outOfLimits(QString)), this,
          SLOT(on_outOfLimits(QString)));
  connect(_machine["Z"], SIGNAL(outOfLimits(QString)), this,
          SLOT(on_outOfLimits(QString)));

  // when INI file loaded, update GUI
  connect(&_machine, SIGNAL(dataHasChanged()), this,
          SLOT(on_machineDataChanged()));

  // GUI
  connect(ui->startButton, SIGNAL(clicked(bool)), this, SLOT(on_startButtonClicked()));

  // Enable signals from GUI form to machine
  toggleFormConections(On);

  // Timed action for reading data from axes
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, [this]() {
    int x = _machine["X"]->count;
    int y = _machine["Y"]->count;
    int z = _machine["Z"]->count;
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
  ui->setPointXSlider->setMaximum(_machine["X"]->length * 1000);
  ui->setPointYSlider->setMaximum(_machine["Y"]->length * 1000);
  ui->setPointZSlider->setMaximum(_machine["Z"]->length * 1000);
  
  ui->xPositionBar->setMaximum(_machine["X"]->length * 1000);
  ui->yPositionBar->setMaximum(_machine["Y"]->length * 1000);
  ui->zPositionBar->setMaximum(_machine["Z"]->length * 1000);
  
  ui->xpSpinBox->setValue(_machine["X"]->p);
  ui->xiSpinBox->setValue(_machine["X"]->i);
  ui->xdSpinBox->setValue(_machine["X"]->d);
  
  ui->ypSpinBox->setValue(_machine["Y"]->p);
  ui->yiSpinBox->setValue(_machine["Y"]->i);
  ui->ydSpinBox->setValue(_machine["Y"]->d);
  
  ui->zpSpinBox->setValue(_machine["Z"]->p);
  ui->ziSpinBox->setValue(_machine["Z"]->i);
  ui->zdSpinBox->setValue(_machine["Z"]->d);
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
}

//   ____  _       _       
//  / ___|| | ___ | |_ ___ 
//  \___ \| |/ _ \| __/ __|
//   ___) | | (_) | |_\__ \
//  |____/|_|\___/ \__|___/
                        

// Update machine when GUI data change
void MainWindow::on_formDataChanged() {
  _machine["X"]->p = ui->xpSpinBox->value();
  _machine["X"]->i = ui->xiSpinBox->value();
  _machine["X"]->d = ui->xdSpinBox->value();
  
  _machine["Y"]->p = ui->ypSpinBox->value();
  _machine["Y"]->i = ui->yiSpinBox->value();
  _machine["Y"]->d = ui->ydSpinBox->value();
  
  _machine["Z"]->p = ui->zpSpinBox->value();
  _machine["Z"]->i = ui->ziSpinBox->value();
  _machine["Z"]->d = ui->zdSpinBox->value();
  
  _machine["X"]->setpoint = ui->setPointXSlider->value();
  _machine["Y"]->setpoint = ui->setPointYSlider->value();
  _machine["Z"]->setpoint = ui->setPointZSlider->value();
}


void MainWindow::on_action_Open_INI_file_triggered() {
  QString fileName =
      QFileDialog::getOpenFileName(this, "Open INI file", "", "");
  if (fileName.isNull())
    return;
  _machine.loadIniFile(fileName);
}

// Start/stop simulation
void MainWindow::on_startButtonClicked() {
  if (!_running) {
    _machine.start();
    _running = true;
    ui->startButton->setText("Stop");
    _machine.timer->start();
  } else {
    _machine.stop();
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

