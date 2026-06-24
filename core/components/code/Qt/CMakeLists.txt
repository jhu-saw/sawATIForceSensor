#
# (C) Copyright 2013-2026 Johns Hopkins University (JHU), All Rights Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---

set (REQUIRED_CISST_LIBRARIES
     cisstCommon
     cisstVector
     cisstVectorQt
     cisstNumerical
     cisstOSAbstraction
     cisstMultiTask
     cisstMultiTaskQt
     cisstParameterTypes
     cisstParameterTypesQt)

# cisst
find_package (cisst COMPONENTS ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND)

  if (CISST_HAS_XML)
    set(REQUIRED_CISST_LIBRARIES ${REQUIRED_CISST_LIBRARIES} cisstCommonXML)
  endif (CISST_HAS_XML)

  # for Qt generated header files
  include_directories (${CMAKE_CURRENT_BINARY_DIR})
  set (CMAKE_AUTOMOC ON)

  add_library (
    sawATIForceSensorQt
    ${sawATIForceSensor_HEADER_DIR}/sawATIForceSensorQtExport.h
    ${sawATIForceSensor_HEADER_DIR}/mtsATINetFTQtWidget.h
    ${sawATIForceSensor_HEADER_DIR}/sawATINetFTSimulatorQtWidget.h
    mtsATINetFTQtWidget.cpp
    sawATINetFTSimulatorQtWidget.cpp)

  set_target_properties (
    sawATIForceSensorQt
    PROPERTIES
    VERSION ${sawATIForceSensor_VERSION}
    FOLDER "sawATIForceSensor")

  if (CISST_HAS_XML)
    target_link_libraries(sawATIForceSensorQt ${cisst_LIBRARIES} cisst::cisstCommonXML)
  else (CISST_HAS_XML)
    target_link_libraries(sawATIForceSensorQt ${cisst_LIBRARIES})
  endif (CISST_HAS_XML)

  # make sure the new library is known by the parent folder to add to the config file
  set (sawATIForceSensorQt_LIBRARIES sawATIForceSensorQt PARENT_SCOPE)

  install (TARGETS sawATIForceSensorQt COMPONENT sawATIForceSensor-Qt
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib)

else (cisst_FOUND)
  message ("Information: code will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND)
