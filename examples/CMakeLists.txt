#
# Author(s):  Preetham Chalasani
# Created on: 2014-08-18
#
# (C) Copyright 2014 Johns Hopkins University (JHU), All Rights
# Reserved.
#
# --- begin cisst license - do not edit ---
#
# This software is provided "as is" under an open source license, with
# no warranty.  The complete license can be found in license.txt and
# http://www.cisst.org/cisst/license.txt.
#
# --- end cisst license ---


# create a list of libraries needed for this project
set( REQUIRED_CISST_LIBRARIES cisstCommon
  cisstOSAbstraction
  cisstMultiTask
  cisstQt
  cisstMultiTaskQt
  cisstVectorQt)

# find cisst and make sure the required libraries have been compiled
FIND_PACKAGE(cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} QUIET)
#FIND_PACKAGE(Qt4 REQUIRED )

if (cisst_FOUND_AS_REQUIRED)

  # load cisst configuration
  INCLUDE(${CISST_USE_FILE})
  #INCLUDE(${QT_USE_FILE})

  # sawATINetFT has been compiled within cisst, we should find it automatically
  cisst_find_saw_component( sawATINetFT REQUIRED )
  
  if( sawATINetFT_FOUND )

    # sawATINetFT configuration
    include_directories( ${sawATINetFT_INCLUDE_DIR} )
    link_directories( ${sawATINetFT_LIBRARY_DIR} )
        

    if( CISST_HAS_QT )
        add_executable( sawATINetFTExample
                        main.cpp                        
                      )

        # link against non cisst libraries and cisst components
        target_link_libraries( sawATINetFTExample                                                              
                               ${sawATINetFT_LIBRARIES}
                             )

        # link against cisst libraries (and dependencies)
        cisst_target_link_libraries( sawATINetFTExample ${REQUIRED_CISST_LIBRARIES} )
        set_property (TARGET sawATINetFTExample PROPERTY FOLDER "sawATINetFT")

    endif( CISST_HAS_QT )

  endif( sawATINetFT_FOUND )

endif( cisst_FOUND_AS_REQUIRED )
