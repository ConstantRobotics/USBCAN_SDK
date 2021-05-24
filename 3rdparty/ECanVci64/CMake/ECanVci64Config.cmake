# Config file for the rf62Xsdk package, defines the following variables
# ECanVci64_INCLUDE_DIRS
# ECanVci64_LIBRARIES
# ECanVci64_LIBRARY_DIRS
# ECanVci64_LIBRARY_TYPE ("SHARED" or "STATIC")

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../" ABSOLUTE)

####################################################################################

set(ECanVci64_INCLUDE_DIRS "${PACKAGE_PREFIX_DIR}/include")

if (ECanVci64_LIBRARY_TYPE STREQUAL "SHARED")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ECanVci64_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/bin/debug")
    else()
        set(ECanVci64_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/bin/release")
    endif()
else()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(ECanVci64_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/lib/debug")
    else()
        set(ECanVci64_LIBRARY_DIRS "${PACKAGE_PREFIX_DIR}/lib/release")
    endif()
endif()

set(ECanVci64_LIBRARIES    "ECanVci64")

