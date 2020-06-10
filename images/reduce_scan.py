#!/usr/bin/python
from sys import *
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from astropy.visualization import astropy_mpl_style
#plt.style.use(astropy_mpl_style)
from astropy.io import fits

scan_hdul = fits.open(argv[1], do_not_scale_image_data=True)
scan_data = scan_hdul[0].data
rows, columns = scan_data.shape
print scan_data.shape
fscan = np.array(scan_data, dtype=np.float32)
scan_hdul.close()

# Normalize array from 0.0 to 1.0
fscan += 32768.0
fscan /= 65535.0

scan_med = np.median(fscan)
for col in xrange(columns) :
    col_med = np.median(fscan[:,col])
    fscan[:,col] -= col_med
    fscan[:,col] *= (1-scan_med)/(1-col_med)
    fscan[:,col] += scan_med
plt.figure()
plt.imshow(fscan, cmap='plasma', norm=mcolors.PowerNorm(0.3))
plt.show()
fscan_hdu = fits.PrimaryHDU(fscan)
fscan_hdu.writeto('reduce_scan.fits')
