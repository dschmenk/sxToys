from sys import *
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from astropy.visualization import astropy_mpl_style
#plt.style.use(astropy_mpl_style)
from astropy.io import fits

s
if len(argv) < 2 :
    print("Usage: ", argv[0], " <FITS file> [reduced FITS file]")
    exit(0)

scan_hdul = fits.open(argv[1], do_not_scale_image_data=True)
scan_data = scan_hdul[0].data
rows, columns = scan_data.shape
print(scan_data.shape)
fscan = np.array(scan_data, dtype=np.float32)
scan_hdul.close()

if fscan.dtype == np.int16 :
    # Normalize array from 0.0 to 1.0
    fscan += 32768.0
    fscan /= 65535.0

# Reduce image based on median (background) level: reduce_pix = (1 - image_median)/(1 - column_median)*(raw_pix - column_median) + image_median
scan_med = np.median(fscan)
for col in range(columns) :
    col_med = np.median(fscan[:,col])
    fscan[:,col] -= col_med
    fscan[:,col] *= (1-scan_med)/(1-col_med)
    fscan[:,col] += scan_med

if len(argv) < 3 :
    reduce_scan = 'reduce_scan.fits'
else :
    reduce_scan = argv[2]
fscan_hdu = fits.PrimaryHDU(fscan)
fscan_hdu.writeto(reduce_scan)

# Plot the result
plt.figure()
plt.imshow(fscan, cmap='plasma', norm=mcolors.PowerNorm(0.3))
plt.show()
