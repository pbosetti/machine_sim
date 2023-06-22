#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "machine.h"
#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_action_Open_INI_file_triggered();
    void on_machineDataChanged();

    void on_startButton_clicked();

private:
    Machine machine;

    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *);
    void dragLeaveEvent(QDragLeaveEvent* event);
    Ui::MainWindow *ui;

    bool _running = false;
};
#endif // MAINWINDOW_H
