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

#ifndef GlASSYCLOCK_H
#define GlASSYCLOCK_H

#include <QWidget>
#include <QSvgRenderer>
#include <QTimer>

namespace GlassyClock {

class GClock : public QWidget {
  Q_OBJECT

public:
  GClock(int size = 0, const QPoint pos = QPoint(-1, -1), const QString &screenName = QString(), QWidget *parent = nullptr);
  ~GClock();

  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void showEvent(QShowEvent *event) override;

private slots:
  void updateClock();

private:
  int size_;
  QPoint pos_;
  QString screenName_;
  QSvgRenderer *renderer_;
  QTimer *timer_;
};

}

#endif // GlASSYCLOCK_H
