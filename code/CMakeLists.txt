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

set(
	REQUIRED_CISST_LIBRARIES
	cisstCommon
	cisstCommonXML
	cisstVector
	cisstOSAbstraction
	cisstMultiTask
	cisstParameterTypes 
	)

FIND_PACKAGE( cisst REQUIRED ${REQUIRED_CISST_LIBRARIES} )

if (cisst_FOUND)

	# load cisst configuration
        INCLUDE(${CISST_USE_FILE})

	# create/configure file for find_package (sawATINetFT)
	set (sawATINetFT_INCLUDE_DIR "${sawATINetFT_SOURCE_DIR}/include")
	set (sawATINetFT_LIBRARY_DIR "${LIBRARY_OUTPUT_PATH}")
	set (sawATINetFT_LIBRARIES sawATINetFT)

	include_directories (${sawATINetFT_INCLUDE_DIR})
	set (sawATINetFT_HEADER_DIR "${sawATINetFT_INCLUDE_DIR}/sawATINetFT")

	set (HEADER_FILES                
		${sawATINetFT_HEADER_DIR}/mtsATINetFTLogger.h
                ${sawATINetFT_HEADER_DIR}/mtsATINetFTSensor.h
                ${sawATINetFT_HEADER_DIR}/mtsATINetFTConfig.h
                ${sawATINetFT_HEADER_DIR}/mtsATINetFTQtWidget
		)

	set (SOURCE_FILES                
		mtsATINetFTLogger.cpp
                mtsATINetFTSensor.cpp
                mtsATINetFTConfig.cpp
		)

	add_library (sawATINetFT ${HEADER_FILES} ${SOURCE_FILES})
	cisst_target_link_libraries (sawATINetFT ${REQUIRED_CISST_LIBRARIES})
	set_property (TARGET sawATINetFT PROPERTY FOLDER "sawATINetFT")

	# # link against non cisst libraries and cisst components
	# target_link_libraries (sawATINetFT ${sawConstraintController_LIBRARIES}
	# 	${sawRobotIO1394_LIBRARIES}
	# 	${sawControllers_LIBRARIES}
	# 	#${orocos_kdl_LIBRARIES}
	# 	)

	# add Qt code
	add_subdirectory (Qt)
        set (sawATINetFT_LIBRARIES ${sawATINetFT_LIBRARIES} ${sawATINetFTQt_LIBRARIES})

	configure_file (
		"${CMAKE_CURRENT_SOURCE_DIR}/sawATINetFTConfig.cmake.in"
		"${sawATINetFT_BINARY_DIR}/sawATINetFTConfig.cmake"
		@ONLY@)

	# Install targets for headers and library
	install (DIRECTORY
		${sawATINetFT_INCLUDE_DIR}/sawATINetFT
		DESTINATION include
		PATTERN .svn EXCLUDE)

	install (TARGETS sawATINetFT
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib)

	install (FILES "${sawATINetFT_BINARY_DIR}/sawATINetFTConfig.cmake"
		DESTINATION "${CISST_CMAKE_INSTALL_SUFFIX}/saw")  

endif (cisst_FOUND)
