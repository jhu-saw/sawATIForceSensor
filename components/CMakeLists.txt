#
# Author(s):  Preetham Chalasani
# Created on: 2014-08-18
#
# (C) Copyright 2014-2016 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

project (sawATIForceSensor)

cmake_minimum_required (VERSION 2.8)

set (REQUIRED_CISST_LIBRARIES
     cisstCommon
     cisstCommonXML
     cisstVector
     cisstOSAbstraction
     cisstMultiTask
     cisstParameterTypes
     )

find_package (cisst 1.1.0 REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # catkin/ROS paths
  cisst_is_catkin_build (sawATIForceSensor_IS_CATKIN_BUILT)
  if (sawATIForceSensor_IS_CATKIN_BUILT)
    set (sawATIForceSensor_CONFIG_FILE_DIR
         "${CATKIN_DEVEL_PREFIX}/share/sawATIForceSensor/cmake")
    set (LIBRARY_OUTPUT_PATH "${sawATIForceSensor_BINARY_DIR}/lib")
  else ()
    set (sawATIForceSensor_CONFIG_FILE_DIR "${sawATIForceSensor_BINARY_DIR}")
  endif ()

  # create/configure file for find_package (sawATIForceSensor)
  set (sawATIForceSensor_INCLUDE_DIR
       "${sawATIForceSensor_SOURCE_DIR}/include"
       "${sawATIForceSensor_BINARY_DIR}/include")
  set (sawATIForceSensor_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  set (sawATIForceSensor_LIBRARIES sawATIForceSensor)

  # Set the version number
  set (sawATIForceSensor_VERSION_MAJOR "1")
  set (sawATIForceSensor_VERSION_MINOR "4")
  set (sawATIForceSensor_VERSION_PATCH "0")
  set (sawATIForceSensor_VERSION "${sawATIForceSensor_VERSION_MAJOR}.${sawATIForceSensor_VERSION_MINOR}.${sawATIForceSensor_VERSION_PATCH}")

  # Generate sawATIForceSensorRevision.h
  configure_file ("${sawATIForceSensor_SOURCE_DIR}/code/sawATIForceSensorRevision.h.in"
                  "${sawATIForceSensor_BINARY_DIR}/include/sawATIForceSensor/sawATIForceSensorRevision.h")

  include_directories (${sawATIForceSensor_INCLUDE_DIR})
  set (sawATIForceSensor_HEADER_DIR "${sawATIForceSensor_SOURCE_DIR}/include/sawATIForceSensor")

  set (HEADER_FILES
       ${sawATIForceSensor_HEADER_DIR}/sawATIForceSensorExport.h
       ${sawATIForceSensor_HEADER_DIR}/mtsATINetFTSensor.h
       ${sawATIForceSensor_HEADER_DIR}/mtsATINetFTConfig.h
       )

  set (SOURCE_FILES
       code/mtsATINetFTSensor.cpp
       code/mtsATINetFTConfig.cpp
       )

  add_library (sawATIForceSensor ${HEADER_FILES} ${SOURCE_FILES})
  cisst_target_link_libraries (sawATIForceSensor ${REQUIRED_CISST_LIBRARIES})
  set_property (TARGET sawATIForceSensor PROPERTY FOLDER "sawATIForceSensor")

  # add Qt code
  if (CISST_HAS_QT)
    add_subdirectory (code/Qt)
    set (sawATIForceSensor_LIBRARIES ${sawATIForceSensor_LIBRARIES} ${sawATIForceSensorQt_LIBRARIES})
  endif (CISST_HAS_QT)

  set (sawATIForceSensor_CMAKE_CONFIG_FILE
       "${sawATIForceSensor_CONFIG_FILE_DIR}/sawATIForceSensorConfig.cmake")

  configure_file ("${CMAKE_CURRENT_SOURCE_DIR}/cmake/sawATIForceSensorConfig.cmake.in"
                  "${sawATIForceSensor_CMAKE_CONFIG_FILE}"
                  @ONLY)

  cisst_add_config_version ("${sawATIForceSensor_CONFIG_FILE_DIR}/sawATIForceSensorConfigVersion.cmake"
                            VERSION ${sawATIForceSensor_VERSION}
                            DESTINATION ${CISST_CMAKE_INSTALL_SUFFIX}/saw
                            COMPONENT sawATIForceSensor)

  # Install targets for headers and library
  install (DIRECTORY
           "${sawATIForceSensor_INCLUDE_DIR}/sawATIForceSensor"
           "${sawATIForceSensor_BINARY_DIR}/include/sawATIForceSensor"
           DESTINATION include
           PATTERN .svn EXCLUDE)

  install (TARGETS sawATIForceSensor
           RUNTIME DESTINATION bin
           LIBRARY DESTINATION lib
           ARCHIVE DESTINATION lib)

  install (FILES "${sawATIForceSensor_CMAKE_CONFIG_FILE}"
           DESTINATION "share/sawATIForceSensor")

endif (cisst_FOUND_AS_REQUIRED)
