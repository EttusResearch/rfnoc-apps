# RFNoC: Apps, example RFNoC out-of-tree blocks and UHD apps

This repo contains useful RFNoC out-of-tree blocks and UHD apps.

If building FPGA images, make sure to use the CMake flag "-DUHD_FPGA_DIR=..." and
point it to your uhd/fpga source directory. This will create make targets for
building FPGA images. For example, you can build a X310 HG image using
`make x310_HG_rfnoc_image_core`. Much sure to check the UHD manual for which
version of Xilinx's Vivado tools you need for building FPGA images.

## Directory Structure

* `apps`: This directory contains example UHD applications for both in-tree RFNoC blocks
  and RFNoC blocks in this repo.

* `blocks`: This directory contains the RFNoC block's block definition YAML files. These
  block definitions can be read by the RFNoC tools, and will get installed into the
  system for use by other out-of-tree modules.

* `cmake`: CMake related files.

* `fpga`: This directory contains the source code for the HDL modules of the
  individual RFNoC blocks, along with their testbenches, and additional modules
  required to build the blocks. There is one subdirectory for every block.

* `icores`: Example RFNoC image core files for building FPGA images.

* `include`: This directory contains all the block controllers' header files.

* `lib`: This directory contains all the block controllers' non-header source
  files for this repo's, which are usually cpp files.
