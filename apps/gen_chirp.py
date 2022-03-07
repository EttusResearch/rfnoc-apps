#!/usr/bin/env python
#
# Copyright 2022 Ettus Research, a National Instruments Brand
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
import matplotlib.pyplot as pyplot

num_points        = 1.0e5        # Number of points to generate
fs                = 1.0e5        # Sampling frequency (Hz)
f_start           = 0            # Start frequency (Hz)
f_end             = fs/8         # End frequency (Hz)
amplitude         = 0.9          # Amplitude
output_filename   = "input.dat"  # Output file name

t = np.arange(0, num_points)*(1.0/fs)
f_sweep = np.linspace(f_start, f_end, num_points, endpoint=False)
samps = amplitude*np.sin(2.0 * np.pi * f_sweep * t) + (1.0j)*amplitude*np.cos(2.0 * np.pi * f_sweep * t)
samps.astype("csingle").tofile(output_filename)

pyplot.plot(samps.real)
pyplot.plot(samps.imag)
pyplot.show()
