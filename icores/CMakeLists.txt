#
# Copyright 2022 Ettus Research, a National Instruments Brand
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

# Grab all image core YAML filenames and register them
file(GLOB icore_yml_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*_rfnoc_image_core.yml")
foreach (icore_yml ${icore_yml_files})
    RFNOC_REGISTER_IMAGE_CORE(SRC ${icore_yml})
endforeach()

