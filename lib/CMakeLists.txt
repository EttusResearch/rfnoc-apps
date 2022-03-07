#
# Copyright 2022 Ettus Research, a National Instruments Brand
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Setup library
########################################################################
# List any C++ sources here. If there are no sources (e.g., because there
# is no block controller), then this directory will be skipped.
list(APPEND rfnoc_apps_sources
    # Nothing for now...
)
if(NOT rfnoc_apps_sources)
    MESSAGE(STATUS "No C++ sources... skipping lib/")
    return()
endif()

########################################################################
# Setup the include and linker paths
########################################################################
include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/lib
    ${CMAKE_BINARY_DIR}/include
    ${UHD_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIR}
)

link_directories(
    ${Boost_LIBRARY_DIRS}
)

add_library(rfnoc-apps SHARED
    ${rfnoc_apps_sources}
)
target_link_libraries(rfnoc-apps
    ${UHD_LIBRARIES}
    ${Boost_LIBRARIES}
    ${GNURADIO_ALL_LIBRARIES}
    ${ETTUS_LIBRARIES}
)
set_target_properties(rfnoc-apps
    PROPERTIES DEFINE_SYMBOL "rfnoc_apps_EXPORTS")

########################################################################
# Install built library files
########################################################################
install(TARGETS rfnoc-apps
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

