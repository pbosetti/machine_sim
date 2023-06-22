#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDragLeaveEvent>
#include <QFileDialog>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setAcceptDrops(true);
    connect(&machine, SIGNAL(dataHasChanged()), this, SLOT(on_machineDataChanged()));

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        qDebug() << "Count: " << machine.x()->count;
    });
    timer->start(500);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_machineDataChanged() {
    ui->brokerAddressField->setText(*machine.brokerAddress());
    ui->brokerPortField->setValue(machine.brokerPort());
    ui->pubTopicField->setText(*machine.pubTopic());
    ui->subTopicField->setText(*machine.subTopic());
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        QList urls = e->mimeData()->urls();
        if (urls.count() != 1) {
            statusBar()->showMessage("One file only, please!");
        } else {
            QUrl url = urls.first();
            if (url.path().endsWith(".ini")) {
                statusBar()->showMessage(QString("Release to open ") + url.toLocalFile(), 10000);
                e->acceptProposedAction();
            } else {
                statusBar()->showMessage("Wrong file type");
            }

        }
    }
}

void MainWindow::dragLeaveEvent(QDragLeaveEvent*)
{
    statusBar()->clearMessage();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    QUrl url = e->mimeData()->urls().first();
    QString fileName = url.toLocalFile();
    statusBar()->showMessage(QString("Opening ") + fileName);
    machine.loadIniFile(fileName);
}


void MainWindow::on_action_Open_INI_file_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open INI file", "","");
    if (fileName.isNull()) return;
    machine.loadIniFile(fileName);
}


void MainWindow::on_startButton_clicked()
{
    if (!_running) {
        machine.start();
        _running = true;
        ui->startButton->setText("Stop");
    } else {
        machine.stop();
        _running = false;
        ui->startButton->setText("Start");
    }

}


