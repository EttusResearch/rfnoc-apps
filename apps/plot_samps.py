#!/usr/bin/env python
#
# Copyright 2022 Ettus Research, a National Instruments Brand
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
import matplotlib.pyplot as pyplot

input_filename  = "input.dat"   # Input file name
output_filename = "output.dat"  # Output file name

in_samps = np.fromfile("input.dat", dtype="csingle")
out_samps = np.fromfile("output.dat", dtype="csingle")

pyplot.plot(in_samps.real)
pyplot.plot(out_samps.real)
pyplot.title("Input and Output Samples (Real)")
pyplot.show()
