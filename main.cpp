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

#include <signal.h>
#include <QApplication>
#include <QPoint>
#include <QTextStream>
#include <QGuiApplication>
#include <QScreen>

#include "glassyclock.h"

// Global variables for screen monitoring
std::unique_ptr<GlassyClock::GClock> g_clock;
int g_size = 0;
QPoint g_pos(-1, -1);
QString g_screenName;

QScreen* getTargetScreen(QString screenName) {
  // Qt adds placeholder screens when no real screen is present
  auto screens = QGuiApplication::screens();
  screens.removeIf([](QScreen *screen) {
    QRect geom = screen->geometry();
    return geom.width() <= 0 || geom.height() <= 0;
  });

  if (screens.isEmpty())
    return nullptr;

  if (!screenName.isEmpty()) {
    for (const auto &screen : screens)
      if (screen->name() == screenName)
        return screen;
    return nullptr;
  }

  // Find the leftmost or topmost screen when no specific screen is requested
  std::sort(screens.begin(), screens.end(), [](QScreen *a1, QScreen *a2) {
    QPoint p1(a1->geometry().topLeft());
    QPoint p2(a2->geometry().topLeft());
    return (qAbs(p1.x() - p2.x()) > qAbs(p1.y() - p2.y()) ? p1.x() < p2.x()
                                                          : p1.y() < p2.y());
  });
  return screens.at(0);
}

void onScreenChanged(QScreen *screen) {
  Q_UNUSED(screen);
  QScreen* targetScreen = getTargetScreen(g_screenName);

  if (g_clock && targetScreen == g_clock->currentTargetScreen())
    return;

  if (g_clock) {
    g_clock->hide();
    g_clock.reset();
  }

  if (targetScreen != nullptr) {
    g_clock = std::make_unique<GlassyClock::GClock>(g_size, g_pos, targetScreen);
    g_clock->show();
  }
}

void handleQuitSignals(const std::vector<int>& quitSignals) {
  auto handler = [](int sig) ->void {
    Q_UNUSED(sig);
    QCoreApplication::quit();
  };

  for(int sig : quitSignals)
    signal(sig, handler);
}

int main(int argc, char *argv[]) {
  const QString name = "glassyclock";
  const QString firstArg = QString::fromUtf8(argv[1]);
  if (firstArg == "--help" || firstArg == "-h") {
    QTextStream out (stdout);
    out << "GlassyClock - A simple, Qt-based, analog clock for any desktop\n\n"\
           "Usage:\n	glassyclock [options]\n\n"\
           "Options:\n\n"\
           "--help or -h     Show this help and exit.\n"\
           "<s>              Clock's size.\n"\
           "<s> <x> <y>      Clock's size and position.\n"\
           "<s> <x> <y> <S>  Clock's size, position and Wayland screen name.\n"\
           "\nNOTE: <X> means X without brackets."
        << Qt::endl;
    return 0;
  }

  QApplication app(argc, argv);
  app.setApplicationName(name);
  app.setQuitOnLastWindowClosed(false);
  handleQuitSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});

  if (argc > 1) {
    bool ok;
    int n = firstArg.toInt(&ok);
    if (ok) {
      g_size = n;
      if (argc > 3) {
        n = QString::fromUtf8(argv[2]).toInt(&ok);
        if (ok) {
          g_pos.setX(n);
          n = QString::fromUtf8(argv[3]).toInt(&ok);
          if (ok)
            g_pos.setY(n);
        }
        if (argc > 4) {
          g_screenName = QString::fromUtf8(argv[4]);
        }
      }
    }
  }

  QObject::connect(qobject_cast<QGuiApplication*>(&app), &QGuiApplication::screenAdded, onScreenChanged);
  QObject::connect(qobject_cast<QGuiApplication*>(&app), &QGuiApplication::screenRemoved, onScreenChanged);

  // We don't get any event for the intial screens it seems, just trigger it once
  onScreenChanged(nullptr);
  return app.exec();
}
