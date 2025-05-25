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

#include "glassyclock.h"

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
  handleQuitSignals({SIGQUIT, SIGINT, SIGTERM, SIGHUP});

  int s = 0;
  QPoint p(-1, -1);
  QString screenName;
  if (argc > 1) {
    bool ok;
    int n = firstArg.toInt(&ok);
    if (ok) {
      s = n;
      if (argc > 3) {
        n = QString::fromUtf8(argv[2]).toInt(&ok);
        if (ok) {
          p.setX(n);
          n = QString::fromUtf8(argv[3]).toInt(&ok);
          if (ok)
            p.setY(n);
        }
        if (argc > 4) {
          screenName = QString::fromUtf8(argv[4]);
        }
      }
    }
  }
  GlassyClock::GClock clock(s, p, screenName);
  clock.show();
  return app.exec();
}
