#!/usr/bin/env python
#
# Copyright 2022 Ettus Research, a National Instruments Brand
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

import numpy as np
import matplotlib.pyplot as pyplot
from gnuradio.filter import pm_remez

num_coeffs        = 41            # Filter length
freq_cutoff       = 0.25          # Normalized frequency [0.0 - 1.0]
freq_trans_width  = 0.10          # Filter transition width, normalized
stopband_atten_dB = -40           # Attenuation in stopband in dB
output_filename   = "coeffs.dat"  # Output filename

stopband_atten = 10**(stopband_atten_dB/20)
coeffs_float   = np.array(pm_remez(num_coeffs-1,[0.0,freq_cutoff-freq_trans_width,freq_cutoff+freq_trans_width,1.0],[1,1,stopband_atten,stopband_atten],[1,1]))
coeffs = np.round(32767.0*coeffs_float).astype("short")
coeffs.tofile(output_filename)

pyplot.plot(coeffs)
pyplot.show()
