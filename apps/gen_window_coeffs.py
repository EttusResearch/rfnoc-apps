#!/usr/bin/env python
#
# Copyright 2022 Ettus Research, a National Instruments Brand
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
import matplotlib.pyplot as pyplot
from gnuradio.fft import window

num_coeffs      = 4096          # Window length
output_filename = "coeffs.dat"  # Output file name

coeffs_float = np.array(window.blackman_harris(num_coeffs))
coeffs = np.round(32767.0*coeffs_float).astype("short")
coeffs.tofile(output_filename)

pyplot.plot(coeffs)
pyplot.show()
