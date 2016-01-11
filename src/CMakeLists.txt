# Target name
SET(TARGET_LIBRARYNAME OsgOpenVR)
SET(TARGET_TARGETNAME_VIEWER OpenVRViewerExample)

# Source files for library
SET(TARGET_SRC
	openvrviewer.cpp
	openvrdevice.cpp
	openvreventhandler.cpp
	openvrupdateslavecallback.cpp
)
# Header files for library
SET(TARGET_H
	openvrviewer.h
	openvrdevice.h
	openvreventhandler.h
	openvrupdateslavecallback.h
)

#####################################################################
# Create library
#####################################################################

ADD_LIBRARY(${TARGET_LIBRARYNAME} ${TARGET_SRC} ${TARGET_H})

#####################################################################
# Linker options
#####################################################################

# Link to OpenSceneGraph libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} ${OPENSCENEGRAPH_LIBRARIES} )

# Link to OpenVR libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} ${OPENVR_SDK_LIBRARIES}	)

# Link to open gl libs
TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} ${OPENGL_LIBRARIES} )

# Link to libraries needed for OpenVR on Visual Studio
IF(MSVC)
	TARGET_LINK_LIBRARIES(${TARGET_LIBRARYNAME} winmm.lib ws2_32.lib)
ENDIF(MSVC)

#####################################################################
# Create executable
#####################################################################
IF(BUILD_EXAMPLES)
	ADD_EXECUTABLE(${TARGET_TARGETNAME_VIEWER} viewerexample.cpp)
	TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME_VIEWER} ${TARGET_LIBRARYNAME})
ENDIF(BUILD_EXAMPLES)

####################################################################
# Create user file for correct environment string
#####################################################################
IF(MSVC)
	IF (OPENSCENEGRAPH_FOUND)
		# Build path
		SET(USERFILE_ENVIRONMENT_DEBUG "PATH=")
		SET(USERFILE_ENVIRONMENT_RELEASE "PATH=")

		# Add OpenSceneGraph to path
		IF (EXISTS "${OSG_DIR}/bin")
			SET(OSG_BIN_DIR "${OSG_DIR}/bin" CACHE PATH "Path where to find OpenSceneGraph binaries")
		ELSE (EXISTS "${OSG_DIR}/bin")
			SET (OSG_BIN_DIR CACHE PATH "Path where to find OpenSceneGraph binaries")
		ENDIF(EXISTS "${OSG_DIR}/bin")
		SET(USERFILE_ENVIRONMENT_DEBUG "${USERFILE_ENVIRONMENT_DEBUG}${OSG_BIN_DIR}")
		SET(USERFILE_ENVIRONMENT_RELEASE "${USERFILE_ENVIRONMENT_RELEASE}${OSG_BIN_DIR}")

		IF (OSG_THIRD_PARTY_DIR)
			# Add third party to path
			SET(USERFILE_ENVIRONMENT_DEBUG "${USERFILE_ENVIRONMENT_DEBUG};${OSG_THIRD_PARTY_DIR}/bin")
			SET(USERFILE_ENVIRONMENT_RELEASE "${USERFILE_ENVIRONMENT_RELEASE};${OSG_THIRD_PARTY_DIR}/bin")
		ELSE (OSG_THIRD_PARTY_DIR)
			MESSAGE("Warning: OpenSceneGraph 3rd Party Directory not found.")
		ENDIF(OSG_THIRD_PARTY_DIR)


	ENDIF (OPENSCENEGRAPH_FOUND)
ENDIF(MSVC)

