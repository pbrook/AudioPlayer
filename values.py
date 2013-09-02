#! /usr/bin/python
# Generate table of output voltage v.s. input value
# for a resistor based DAC.  Suggested use:
# ./values.py | gnuplot -persist -e "plot '-', x/31"

import io
import sys

rvals = [22.0, 10.0, 5.6, 2.6, 1.3]

num_r = len(rvals)
inv = [1/x for x in rvals]
maxval = (1 << num_r) - 1
fmt = "0x%02x, %f\n"
f = sys.stdout
f.write(fmt % (0, 0.0))
for v in xrange(1, maxval):
    inv_low = 0.0
    inv_high = 0.0
    for i in xrange(0, num_r):
        if (v & (1 << i)) == 0:
            inv_low += inv[i]
        else:
            inv_high += inv[i]
    low = 1.0 / inv_low
    high = 1.0 / inv_high
    res = low / (low + high)
    f.write(fmt % (v, res))

f.write(fmt % (maxval, 1.0))
