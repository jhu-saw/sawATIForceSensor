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

cmake_minimum_required(VERSION 3.10)
project (sawATIForceSensorCore VERSION 2.0.0)

find_package (cisst REQUIRED)
include (${CISST_USE_FILE})
cisst_cpack_settings (
  VENDOR "JHU"
  MAINTAINER "anton.deguet@jhu.edu")

add_subdirectory (components)

if (CISST_HAS_QT)
  set (sawATIForceSensor_DIR "${sawATIForceSensorCore_BINARY_DIR}/components")
  add_subdirectory (examples)
endif ()

include (CPack)
cpack_add_component (sawATIForceSensor)
cpack_add_component (sawATIForceSensor-dev
                     DEPENDS sawATIForceSensor)
if (CISST_HAS_QT)
  cpack_add_component (sawATIForceSensor-Examples
                       DEPENDS sawATIForceSensor)
endif ()
