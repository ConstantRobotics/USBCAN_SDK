# Config file for the rf62Xsdk package, defines the following variables
# ControlCAN_INCLUDE_DIRS
# ControlCAN_LIBRARIES
# ControlCAN_LIBRARY_DIRS
# ControlCAN_LIBRARY_TYPE ("SHARED" or "STATIC")

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

####################################################################################

set(ControlCAN_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include")

if (ControlCAN_LIBRARY_TYPE STREQUAL "SHARED")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ControlCAN_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/bin/debug")
    else()
        set(ControlCAN_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/bin/release")
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ControlCAN_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/lib/debug")
    else()
        set(ControlCAN_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/lib/release")
    endif()
endif()

set(ControlCAN_LIBRARIES    "ControlCAN")

