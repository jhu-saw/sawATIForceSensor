#
# Author(s):  Preetham Chalasani
# Created on: 2014-08-18
#
# (C) Copyright 2014-2026 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

cmake_minimum_required (VERSION 3.10)
project (sawATIForceSensorExamples VERSION 2.1.0)

# create a list of libraries needed for this project
set (REQUIRED_CISST_LIBRARIES
     cisstCommon
     cisstCommonQt
     cisstOSAbstraction
     cisstVector
     cisstMultiTask
     cisstMultiTaskQt
     cisstVectorQt)

# find cisst and make sure the required libraries have been compiled
find_package (cisst COMPONENTS ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND)

  # catkin/ROS paths
  cisst_set_output_path ()

  find_package (sawATIForceSensor REQUIRED)

  if (sawATIForceSensor_FOUND)

    # sawATIForceSensor configuration
    include_directories (${sawATIForceSensor_INCLUDE_DIR})
    link_directories (${sawATIForceSensor_LIBRARY_DIR})

    if (CISST_HAS_QT)

      add_executable (
	sawATIForceSensorExample
        main.cpp)
      target_link_libraries (
	sawATIForceSensorExample
        ${sawATIForceSensor_LIBRARIES}
	${cisst_LIBRARIES})
      set_target_properties (
	sawATIForceSensorExample PROPERTIES
        FOLDER "sawATIForceSensor")

      add_executable (
	sawATIForceSensorSimulator
        mainSimulator.cpp)
      target_link_libraries (
	sawATIForceSensorSimulator
        ${sawATIForceSensor_LIBRARIES}
	${cisst_LIBRARIES})
      set_target_properties (
	sawATIForceSensorSimulator PROPERTIES
        FOLDER "sawATIForceSensor")

      add_executable (sawATIForceSensorWithSimulator
        mainForceWithSimulator.cpp)
      target_link_libraries (sawATIForceSensorWithSimulator
        ${sawATIForceSensor_LIBRARIES}
	${cisst_LIBRARIES})
      set_target_properties (
	sawATIForceSensorWithSimulator PROPERTIES
        FOLDER "sawATIForceSensor")

      install (TARGETS sawATIForceSensorExample sawATIForceSensorSimulator sawATIForceSensorWithSimulator
	COMPONENT sawATIForceSensor-Examples
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

    endif (CISST_HAS_QT)

  endif (sawATIForceSensor_FOUND)

endif (cisst_FOUND)
