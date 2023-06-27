#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "machine.h"
#include <QMainWindow>

enum OnOff { On, Off };

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void on_action_Open_INI_file_triggered();
  void on_machineDataChanged();

  void on_startButton_clicked();
  void on_formDataChanged();
  void on_outOfLimits(QString const &name);

private:
  Machine machine;

  void dragEnterEvent(QDragEnterEvent *e);
  void dropEvent(QDropEvent *);
  void dragLeaveEvent(QDragLeaveEvent *event);
  void syncData();
  void toggleFormConections(enum OnOff);
  Ui::MainWindow *ui;

  bool _running = false;
};
#endif // MAINWINDOW_H
