# GlassyClock

## Overview

GlassyClock is a simple, translucent, Qt-based, analog clock for X11 and Wayland desktops.

On X11, it needs compositing. On Wayland, the name of the screen can be added to the command-line. See `glassyclock --help` for options.

GlassyClock uses KWin's blur effect if available and enabled but does not depend on KWin.

There is no plan for adding GUI options to GlassyClock. If you want to change its appearance, you will need to edit its SVG image and/or code.

## Installation

Qt6 ≥ 6.3, Qt6-SVG ≥ 6.3, LayerShellQt ≥ 6 and KF6WindowSystem ≥ 6 are required.

To compile it, issue these commands inside its source directory:
```sh
mkdir build && cd build
cmake ..
make
```
And if you want to install it, do
```sh
sudo make install
```
