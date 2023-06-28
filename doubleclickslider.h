#ifndef DOUBLECLICKSLIDER_H
#define DOUBLECLICKSLIDER_H

#include <QSlider>
#include <QMouseEvent>
#include <QStyleOption>
#include <QDebug>
#include <QTimer>

class DoubleClickSlider : public QSlider {
  Q_OBJECT
public:
  DoubleClickSlider(QWidget* parent = nullptr) : QSlider(parent) { };

signals:
  void sliderHandleDoubleClicked();

protected:
  void mouseDoubleClickEvent(QMouseEvent *event) override {
    QStyleOptionSlider opt;
    this->initStyleOption(&opt);
    QRect sr = this->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    if (sr.contains(event->pos())) {
      qDebug() << "Double clicked handle";
      QTimer::singleShot(250, this, [=]() {setValue(maximum() / 2.0);});
      emit sliderHandleDoubleClicked();
    }
    QSlider::mouseDoubleClickEvent(event);

  }
};

#endif // DOUBLECLICKSLIDER_H
