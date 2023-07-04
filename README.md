# Machine Simulator for C-CNC

This is a machine simulator and chart viewer for the C-CNC program, as per <https://github.com/pbosetti/c-cnc23>

It is written in QT 6.5.1. If you have Qt installed, you can clone and build it on Mac, Linux or Windows.

For Windows users, you can download a pre-built binary in the [latest releases section](https://github.com/pbosetti/machine_sim/releases).


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