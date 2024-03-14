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

For Linux users, we strongly recommend to compile with ROS (1 or 2).  See https://github.com/jhu-saw/vcs for download and build instructions.  Use the VCS files for `ati-ft`.

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

The ROS node is `ati_ft` and can be found in the package `ati_ft`.  To start a node without any specific namespace:
```
rosrun ati_ft ati_ft -i 192.168.0.2
```

If you have more than one force sensor, you can start the node with a ROS namespace using (you can use your own namespace after `__ns:=`):
```sh
rosrun ati_ft_ros ati_ft -i 192.168.0.2 __ns:=force_sensor_A
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
