# sawATIForceSensor

This SAW component contains code for interfacing with the network based ATI force sensors.  It compiles on Windows and Linux.  It has been tested with:
  * Linux Ubuntu 16.04, 18.04 and 20.04 and Windows
  * ATI Nano/Gamma/Sigma with *Net Box*

The `ros` folder contains code for a ROS node that interfaces with the sawATIForceSensor component and publishes the measured wrench (`measured_cf`).  To build the ROS node, make sure you use `catkin build`.


# Links
 * License: http://github.com/jhu-cisst/cisst/blob/master/license.txt
 * JHU-LCSR software: http://jhu-lcsr.github.io/software/

# Dependencies
 * cisst libraries: https://github.com/jhu-cisst/cisst
 * Qt for user interface
 * ROS (optional)

# Running the examples

## Compilation

This code is part of the cisst-SAW libraries and components.  Once the drivers are installed, you can follow the *cisst-SAW* compilation instructions: https://github.com/jhu-cisst/cisst/wiki/Compiling-cisst-and-SAW-with-CMake.

For Linux users, we strongly recommend to compile with ROS and the python catkin build tools (i.e. `catkin build`, NOT `catkin_make`).  Detailled instructions can be found on https://github.com/jhu-cisst/cisst/wiki/Compiling-cisst-and-SAW-with-CMake#13-building-using-catkin-build-tools-for-ros.

Short version for Ubuntu (18.04) ROS (melodic) to compile using `catkin` and `wstool`:
```sh
sudo apt install libxml2-dev libraw1394-dev libncurses5-dev qtcreator swig sox espeak cmake-curses-gui cmake-qt-gui git subversion gfortran libcppunit-dev libqt5xmlpatterns5-dev # most system dependencies we need
sudo apt install python-wstool python-catkin-tools # catkin and wstool for ROS build
source /opt/ros/melodic/setup.bash # or use whatever version of ROS is installed!
mkdir ~/catkin_ws # create the catkin workspace
cd ~/catkin_ws # go in the workspace
wstool init src # we're going to use wstool to pull all the code from github
catkin init # create files for catkin build tool
catkin config --cmake-args -DCMAKE_BUILD_TYPE=Release # all code should be compiled in release mode
cd src # go in source directory to pull code
wstool merge https://raw.githubusercontent.com/jhu-saw/sawATIForceSensor/master/ros/atift.rosinstall
wstool up # now wstool knows which repositories to pull, let's get the code
catkin build # ... and finally compile everything
```

## Main example

The main example provided is `sawATIForceSensorExample`.  The command line options are:
```sh
sawATIForceSensorExample:
 -c <value>, --configuration <value> : XML configuration file (optional)
 -i <value>, --ftip <value> : Force sensor IP address (optional)
 -p <value>, --customPort <value> : Custom Port Number (optional)
 -t <value>, --timeout <value> : Socket send/receive timeout (optional)
 -m, --component-manager : JSON files to configure component manager (optional)
 -D, --dark-mode : replaces the default Qt palette with darker colors (optional)
```

To use a force sensor identified by its IP, use:
```sh
sawATIForceSensorExample -i 192.168.0.2
```

## ROS

### atinetft_xml node

The ROS node is `atinetft_xml` and can be found in the package `atinetft_ros`.  To start a node without any specific namespace:
```
rosrun atinetft_ros atinetft_xml -i 192.168.0.2
```

If you have more than one force sensor, you can start the node with a ROS namespace using (you can use your own namespace after `__ns:=`):
```sh
rosrun atinetft_ros atinetft_xml -i 192.168.0.2 __ns:=force_sensor_A 
```

Once the node is started AND connected, the following ROS topic should appear:
```sh
/measured_cf
```

Or, if you have specified a namespace:
```sh
/force_sensor_A/measured_cf
```

### ROS CRTK Python and Matlab client

Once you have the ATI Force Sensor ROS node working, you can create your own ROS subscriber in different languages, including C++, Python, Matlab...  If you want to use Python or Matlab, the CRTK client libraries might be useful:
* [CRTK Python](https://github.com/collaborative-robotics/crtk_python_client)
* [CRTK Matlab](https://github.com/collaborative-robotics/crtk_matlab_client)

## Other "middleware"

Besides ROS, the ATI Force Sensor component can also stream data to your application using the *sawOpenIGTLink* or *sawSocketStreamer* components.  See:
* [sawOpenIGTLink](https://github.com/jhu-saw/sawOpenIGTLink)
* [sawSocketStreamer](https://github.com/jhu-saw/sawSocketStreamer)
