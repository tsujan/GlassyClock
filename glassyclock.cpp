/*
 * Copyright (C) Pedram Pourang (aka Tsu Jan) 2023 <tsujan2000@gmail.com>
 *
 * GlassyClock is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GlassyClock is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @license GPL-3.0+ <https://spdx.org/licenses/GPL-3.0+.html>
 */

#include "glassyclock.h"

#include <QGuiApplication>
#include <QPainter>
#include <QTime>
#include <QWindow>
#include <QScreen>

#include <KWindowEffects>
#include <KWindowSystem>
#include <kx11extras.h>

#include <LayerShellQt/Shell>
#include <LayerShellQt/Window>

namespace GlassyClock {

GClock::GClock(int size, const QPoint pos, const QString &screenName , QWidget *parent)
  : QWidget(parent),
    size_(size),
    pos_(pos),
    screenName_(screenName) {
  /* WARNING: Setting "Qt::WindowDoesNotAcceptFocus" might disable
              global shortcuts if no other window is focused. */
  setWindowFlags(Qt::WindowTransparentForInput | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setWindowTitle(QStringLiteral("Glassy Analog Clock"));
  setFixedSize(sizeHint());

  renderer_ = new QSvgRenderer(QStringLiteral(":/Image/clock-face.svg"), this);

  timer_ = new QTimer(this);
  connect(timer_, &QTimer::timeout, this, &GClock::updateClock);
  // start the timer exactly at the start of the second
  QTimer::singleShot(1000 - QTime::currentTime().msec(), Qt::PreciseTimer, this, [this] {
    timer_->start(1000);
  });
}

GClock::~GClock() {
  if (QWindow *win = windowHandle())
    KWindowEffects::enableBlurBehind(win, false);
}

void GClock::updateClock() {
  // ensure that the clock remains accurate within 250 milliseconds (also see paintEvent)
  int msec = QTime::currentTime().msec();
  if (msec > 250 && msec <= 750) {
    timer_->stop();
    QTimer::singleShot(1000 - msec, Qt::PreciseTimer, this, [this] {
      update();
      timer_->start(1000);
    });
  }
  update();
}

void GClock::paintEvent(QPaintEvent *) {
  static const QColor hourColor(255, 255, 255, 210);
  static const QColor secondColor(255, 0, 0, 153);

  int _size = qMin(width(), height());
  QTime time = QTime::currentTime();

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // face
  renderer_->render(&painter);

  painter.translate(width() / 2, height() / 2);
  painter.scale(_size / 200.0, _size / 200.0);

  // border
  QRadialGradient borderGrad(QPointF(-20, -90), 180);
  borderGrad.setColorAt(0, QColor::fromRgbF(0.4, 0.4, 0.4, 0.95));
  borderGrad.setColorAt(0.4, QColor::fromRgbF(0.35, 0.35, 0.35, 0.95));
  borderGrad.setColorAt(1, QColor::fromRgbF(0.3, 0.3, 0.3, 0.95));
  painter.setBrush(Qt::transparent);
  painter.setPen(QPen(borderGrad, 2));
  painter.drawEllipse(QPoint(0, 0), 97, 97);

  // hour hand
  painter.save();
  painter.setPen(Qt::NoPen);
  painter.setBrush(hourColor);
  painter.rotate(30.0 * ((time.hour() + time.minute() / 60.0)));
  painter.drawRoundedRect(-3, -50, 6, 42, 3, 3);
  painter.restore();

  // marks
  painter.setPen(QPen(hourColor, 4));
  for (int i = 0; i < 12; ++i) {
    painter.drawLine(QPointF(88, 0), QPointF(90, 0));
    painter.rotate(30.0);
  }

  // minute hand
  painter.save();
  painter.setPen(Qt::NoPen);
  painter.setBrush(hourColor);
  painter.rotate(6.0 * (time.minute() + time.second() / 60.0));
  painter.drawRoundedRect(-3, -75, 6, 67, 3, 3);
  painter.restore();

  // second hand
  painter.save();
  painter.setPen(Qt::NoPen);
  painter.setBrush(secondColor);
  // round it to the nearest second
  painter.rotate(6.0 * (time.second() + (time.msec() >= 500 ? 1 : 0)));
  painter.drawRoundedRect(-2, -80, 4, 72, 2, 2);
  painter.restore();

  // central circle
  painter.save();
  painter.setPen(QPen(hourColor, 4));
  painter.setBrush(Qt::transparent);
  painter.drawEllipse(-9, -9, 18, 18);
  painter.restore();

  // blur effect
  QWindow *win = windowHandle();
  if (win == nullptr) return;
  qreal f = 0.02 * _size;
  QRectF r((width() - _size + f) / 2, (height() - _size + f) / 2, _size - f - 1, _size - f - 1);
  KWindowEffects::enableBlurBehind(win, true, QRegion(r.toRect(), QRegion::Ellipse));
}

void GClock::resizeEvent(QResizeEvent* /*event*/) {
  int _size = qMin(width(), height());
  QRegion maskedRegion(width() / 2 - _size / 2, height() / 2 - _size / 2, _size, _size,
                       QRegion::Ellipse);
  setMask(maskedRegion);
}

QSize GClock::sizeHint() const {
  int s = qMax(size_, 100);
  return QSize(s, s);
}

void GClock::showEvent(QShowEvent *event) {
  if (QString::compare(QGuiApplication::platformName(), "xcb", Qt::CaseInsensitive) == 0) {
    if (internalWinId()) {
      KX11Extras::setState(internalWinId(),
                           NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher | NET::KeepBelow);
      KX11Extras::setOnDesktop(internalWinId(), -1);
    }

    if (pos_.x() >= 0 && pos_.y() >= 0) {
      if (QWindow *win = windowHandle()) {
        if (QScreen *sc = win->screen()) {
          QRect ag = sc->availableGeometry();
          pos_.setX(qMax(ag.left(), qMin(pos_.x(), ag.right() + 1 - qMax(size_, 100))));
          pos_.setY(qMax(ag.top(), qMin(pos_.y(), ag.bottom() + 1 - qMax(size_, 100))));
        }
      }
      move(pos_);
    }
  }
  else if (QString::compare(QGuiApplication::platformName(), "wayland", Qt::CaseInsensitive) == 0) {
    winId();
    if (QWindow* win = windowHandle()) {
      if (LayerShellQt::Window* layershell = LayerShellQt::Window::get(win)) {
        layershell->setLayer(LayerShellQt::Window::Layer::LayerBottom);
        LayerShellQt::Window::Anchors anchors = {LayerShellQt::Window::AnchorTop
                                                 | LayerShellQt::Window::AnchorLeft};
        layershell->setAnchors(anchors);
        layershell->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
        layershell->setExclusiveZone(-1);
        layershell->setScope(QStringLiteral("glassyclock"));

        // screen
        QScreen *_screen = nullptr;
        auto screens = QGuiApplication::screens();
        if (!screenName_.isEmpty()) {
          for (const auto &screen : std::as_const(screens)) {
            if (screen->name() == screenName_) {
              _screen = screen;
              break;
            }
          }
        }
        if (_screen == nullptr && !screens.isEmpty()) {
          // find the leftmost or topmost screen
          std::sort(screens.begin(), screens.end(), [](QScreen *a1, QScreen *a2) {
            QPoint p1(a1->geometry().topLeft());
            QPoint p2(a2->geometry().topLeft());
            return (qAbs(p1.x() - p2.x()) > qAbs(p1.y() - p2.y()) ? p1.x() < p2.x()
                                                                  : p1.y() < p2.y());
          });
          _screen = screens.at(0);
        }
        if (_screen != nullptr) {
          win->setScreen(_screen);
          layershell->setScreenConfiguration(LayerShellQt::Window::ScreenConfiguration::ScreenFromQWindow);
        }

        // position
        if (pos_.x() >= 0 && pos_.y() >= 0) {
          if (QScreen *sc = win->screen()) {
            QRect g = sc->geometry();
            QRect ag = sc->availableGeometry();
            int left = ag.left() - g.left();
            int top = ag.top() - g.top();
            pos_.setX(qMax(left, qMin(pos_.x(), left + ag.width() - qMax(size_, 100))));
            pos_.setY(qMax(top, qMin(pos_.y(), top + ag.height() - qMax(size_, 100))));
          }
          layershell->setMargins(QMargins(pos_.x(), pos_.y(), 0, 0));
        }
      }
    }
  }
  QWidget::showEvent(event);
}

}
