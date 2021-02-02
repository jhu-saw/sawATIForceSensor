#
# Author(s):  Preetham Chalasani
# Created on: 2014-08-18
#
# (C) Copyright 2014-2021 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

project (sawATIForceSensorExamples)

cmake_minimum_required(VERSION 2.8)

# create a list of libraries needed for this project
set (REQUIRED_CISST_LIBRARIES
     cisstCommon
     cisstOSAbstraction
     cisstVector
     cisstMultiTask
     cisstQt
     cisstMultiTaskQt
     cisstVectorQt)

# find cisst and make sure the required libraries have been compiled
find_package (cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} QUIET)

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # catkin/ROS paths
  cisst_is_catkin_build (sawATIForceSensorExamples_IS_CATKIN_BUILT)
  if (sawATIForceSensorExamples_IS_CATKIN_BUILT)
    set (EXECUTABLE_OUTPUT_PATH "${CATKIN_DEVEL_PREFIX}/bin")
  endif ()

  find_package (sawATIForceSensor REQUIRED)

  if (sawATIForceSensor_FOUND)

    # sawATIForceSensor configuration
    include_directories (${sawATIForceSensor_INCLUDE_DIR})
    link_directories (${sawATIForceSensor_LIBRARY_DIR})

    if (CISST_HAS_QT)
      add_executable (sawATIForceSensorExample
                      main.cpp)

      # link against non cisst libraries and cisst components
      target_link_libraries (sawATIForceSensorExample
                             ${sawATIForceSensor_LIBRARIES})

      # link against cisst libraries (and dependencies)
      cisst_target_link_libraries (sawATIForceSensorExample ${REQUIRED_CISST_LIBRARIES})
      set_property (TARGET sawATIForceSensorExample PROPERTY FOLDER "sawATIForceSensor")

      add_executable (sawATIForceSensorSimulator
                      mainSimulator.cpp)

      # link against non cisst libraries and cisst components
      target_link_libraries (sawATIForceSensorSimulator
                             ${sawATIForceSensor_LIBRARIES})

      # link against cisst libraries (and dependencies)
      cisst_target_link_libraries (sawATIForceSensorSimulator ${REQUIRED_CISST_LIBRARIES})
      set_property (TARGET sawATIForceSensorSimulator PROPERTY FOLDER "sawATIForceSensor")

      add_executable (sawATIForceSensorWithSimulator
                      mainForceWithSimulator.cpp)

      # link against non cisst libraries and cisst components
      target_link_libraries (sawATIForceSensorWithSimulator
                            ${sawATIForceSensor_LIBRARIES})

      # link against cisst libraries (and dependencies)
      cisst_target_link_libraries (sawATIForceSensorWithSimulator ${REQUIRED_CISST_LIBRARIES})
      set_property (TARGET sawATIForceSensorWithSimulator PROPERTY FOLDER "sawATIForceSensor")


    endif (CISST_HAS_QT)

  endif (sawATIForceSensor_FOUND)

endif (cisst_FOUND_AS_REQUIRED)
