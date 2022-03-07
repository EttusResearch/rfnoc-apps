#
# Copyright 2022 Ettus Research, a National Instruments Brand
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

# This app needs Boost
set(BOOST_REQUIRED_COMPONENTS
    program_options
    system
)
if(MSVC)
    set(BOOST_ALL_DYN_LINK "${BOOST_ALL_DYN_LINK}" CACHE BOOL "boost enable dynamic linking")
    if(BOOST_ALL_DYN_LINK)
        add_definitions(-DBOOST_ALL_DYN_LINK) #setup boost auto-linking in msvc
    else(BOOST_ALL_DYN_LINK)
        set(BOOST_REQUIRED_COMPONENTS) #empty components list for static link
    endif(BOOST_ALL_DYN_LINK)
endif(MSVC)
find_package(Boost 1.65 REQUIRED ${BOOST_REQUIRED_COMPONENTS})

find_package(Threads REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
)

set(example_sources
    rfnoc_fir_filter_samples_from_file.cpp
    rfnoc_window_samples_from_file.cpp
    rfnoc_siggen_example.cpp
)   

#for each source: build an executable and install
foreach(example_source ${example_sources})
    get_filename_component(example_name ${example_source} NAME_WE)
    add_executable(${example_name} ${example_source})
    #target_link_libraries(${example_name} ${UHD_LIBRARIES} ${Boost_LIBRARIES} -Wl,--no-as-needed rfnoc-apps) # No lib yet...
    target_link_libraries(${example_name} ${UHD_LIBRARIES} ${Boost_LIBRARIES} -Wl,--no-as-needed -pthread)
    install(TARGETS ${example_name} RUNTIME DESTINATION ${PKG_LIB_DIR}/examples COMPONENT apps)
endforeach(example_source)

