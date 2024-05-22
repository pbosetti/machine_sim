# Machine Simulator for C-CNC

This is a machine simulator and chart viewer for the C-CNC program, as per <https://github.com/pbosetti/c-cnc23>

It is written in QT 6.5.1. If you have Qt installed, you can clone and build it on Mac, Linux or Windows.

For Windows users, you can download a pre-built binary in the [latest releases section](https://github.com/pbosetti/machine_sim/releases).

For Mac users, proceed as follows:

```bash
brew install qt6
git clone https://github.com/pbosetti/machine_sim.git
cd machine_sim
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

then launch the `machine_sim.app` bundle in the `build` folder.

For Linux users (Ubuntu 22.04), proceed as follows to install the prerequisites:

```bash
cd ~
export QTV=6.5.1
sudo apt install python3 python3-pip libgl1-mesa-dev
pip install -U pip
pip install aqtinstall
export PATH=$PATH:$HOME/.local/bin
aqt install-qt linux desktop ${QTV} -m all -O QT
cd ~/Devel
git clone https://github.com/qt/qtmqtt.git
cd qtmqtt
git checkout v${QTV}
export QT6_INSTALL=${HOME}/QT/${QTV}/gcc_64
export Qt6_DIR=${QT6_INSTALL}/lib/cmake/Qt6
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=${QT6_INSTALL}
cmake --build build -j4
cmake --install build
```

Ten the simulator can be build as follows:

```bash
cd ~/Devel
git clone https://github.com/pbosetti/machine_sim.git
cd machine_sim
export Qt6CoreTools_DIR=${QT6_INSTALL}/lib/cmake/Qt6CoreTools
export Qt6WidgetsTools_DIR=${QT6_INSTALL}/lib/cmake/Qt6WidgetsTools
export Qt6Widgets_DIR=${QT6_INSTALL}/lib/cmake/Qt6Widgets
export Qt6GuiTools_DIR=${QT6_INSTALL}/lib/cmake/Qt6GuiTools
QT_DIR=$Qt6_DIR cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

When the repo is updated, you can rebuild the simulator by issuing the following commands:

```bash
cd ~/Devel/machine_sim
git reset --hard
git pull
export Qt6CoreTools_DIR=${QT6_INSTALL}/lib/cmake/Qt6CoreTools
export Qt6WidgetsTools_DIR=${QT6_INSTALL}/lib/cmake/Qt6WidgetsTools
export Qt6Widgets_DIR=${QT6_INSTALL}/lib/cmake/Qt6Widgets
export Qt6GuiTools_DIR=${QT6_INSTALL}/lib/cmake/Qt6GuiTools
QT_DIR=$Qt6_DIR cmake -Bbuild -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
```

Note that the `export` commands are not persistent and must be issued every time you open a new terminal window.

The executable will be in the `build` folder.


## Usage

Launch the executable `machine_sim.exe` (on Windows, without moving it out of the original folder: it needs all that stuff!)

Drag and drop a `machine.ini` file on top of the windows: this will load all the parameters defined by C-CNC, including MQTT communication parameters.

Click on the Start button for running the simulation: you can drag the setpoint sliders and see the resulting motion of the corresponding axis in the progress bar and on the two charts. The simulation runs in real time.

You can enable a bang-bang automatic setting of the setpoint, independently for each axis: this is useful if you want to change the PID parameters and see the result. This is also useful for PID tuning.

Note that the PID parameter changes **are not saved to the original INI file**: if you are tuning the PIDs, once you are satisfied about the step response, manually copy the values back to the original INI file.

You can double click a slider handle for setting the setpoint to half the axis length.

**NOTE**: if one of the axes goes out of the range (i.e. form 0 to axis length), the simulation stops and must be manually restarted, as it would happen in a real machine.

You can drag and zoom on the charts by using mouse and mouse wheel, respectively.

To connect the viewer to the MQTT broker, veryfy that port and address are correct, then click on the connect button. Now you can launch the C-CNC program and see the traces of setpoints and actual axes positions.



## Author

Paolo Bosetti at unitn dot it.