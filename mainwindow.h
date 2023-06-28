#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "machine.h"
#include "qcustomplot.h"
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
  // Lifecycle
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private:
  // Attributes
  Ui::MainWindow *ui;
  Machine _machine;
  bool _running = false;

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
  void on_pushButton_clicked();
};
#endif // MAINWINDOW_H
