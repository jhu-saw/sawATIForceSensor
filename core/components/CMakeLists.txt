#
# Author(s):  Preetham Chalasani
# Created on: 2014-08-18
#
# (C) Copyright 2014-2023 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

cmake_minimum_required (VERSION 3.10)
project (sawATIForceSensor VERSION 2.0.0)

set (REQUIRED_CISST_LIBRARIES
     cisstCommon
     cisstVector
     cisstOSAbstraction
     cisstMultiTask
     cisstParameterTypes
     )

find_package (cisst 1.2.0 REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  include (${CISST_USE_FILE})

  # catkin/ROS paths
  cisst_set_output_path ()

  # create/configure file for find_package (sawATIForceSensor)
  set (sawATIForceSensor_INCLUDE_DIR
       "${sawATIForceSensor_SOURCE_DIR}/include"
       "${sawATIForceSensor_BINARY_DIR}/include")
  set (sawATIForceSensor_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
  set (sawATIForceSensor_LIBRARIES sawATIForceSensor)

  include_directories (BEFORE ${sawATIForceSensor_INCLUDE_DIR})
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

  if (CISST_HAS_XML)
    set(REQUIRED_CISST_LIBRARIES ${REQUIRED_CISST_LIBRARIES} cisstCommonXML)
  else (CISST_HAS_XML)
    message ("Information: sawATIForceSensor compiled without XML support -- cannot use XML config files")
  endif (CISST_HAS_XML)

  add_library (sawATIForceSensor ${HEADER_FILES} ${SOURCE_FILES})

  set_target_properties (sawATIForceSensor PROPERTIES
                         VERSION ${sawATIForceSensor_VERSION}
                         FOLDER "sawATIForceSensor")

  cisst_target_link_libraries (sawATIForceSensor ${REQUIRED_CISST_LIBRARIES})

  # add Qt code
  if (CISST_HAS_QT)
    add_subdirectory (code/Qt)
    set (sawATIForceSensor_LIBRARIES ${sawATIForceSensor_LIBRARIES} ${sawATIForceSensorQt_LIBRARIES})
  endif (CISST_HAS_QT)

  # add all config files for this component
  cisst_add_config_files (sawATIForceSensor)

  # Install targets for headers and library
  install (DIRECTORY
           "${sawATIForceSensor_SOURCE_DIR}/include/sawATIForceSensor"
           "${sawATIForceSensor_BINARY_DIR}/include/sawATIForceSensor"
           DESTINATION include
	   COMPONENT sawATIForceSensor-dev)

  install (TARGETS sawATIForceSensor COMPONENT sawATIForceSensor
           RUNTIME DESTINATION bin
           LIBRARY DESTINATION lib
           ARCHIVE DESTINATION lib)

else (cisst_FOUND_AS_REQUIRED)
  message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)
