from sys import *
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors
from mpl_toolkits.axes_grid1.axes_rgb import RGBAxes
from astropy.visualization import astropy_mpl_style
from astropy.io import fits

def fill_masked_frame(masked_frame) :
    pad_frame = np.pad(masked_frame, 1, 'reflect')
    vfill     = pad_frame[0:-1] + pad_frame[1:]
    hfill      = vfill[:,0:-1]  + vfill[:,1:]
    return hfill[:-1,:-1]

if len(argv) < 2 :
    print("Usage: ", argv[0], " <RAW FITS IMAGE file> [DARK FITS IMAGE file] [colorized file]")
    exit(0)

raw_frame   = fits.getdata(argv[1])
if len(argv) > 2 :
    raw_frame  -= fits.getdata(argv[2])
green_mask = np.array([[0, 0],
                       [0, 1],
                       [0, 0],
                       [1, 0]])
green_frame = fill_masked_frame(raw_frame * np.tile(green_mask, (int(580/4),int(752/2)))) / 65536.0
yellow_mask = np.array([[1, 0],
                        [0, 0],
                        [1, 0],
                        [0, 0]])
yellow_frame = fill_masked_frame(raw_frame * np.tile(yellow_mask, (int(580/4),int(752/2)))) / 65536.0
cyan_mask = np.array([[0, 1],
                      [0, 0],
                      [0, 1],
                      [0, 0]])
cyan_frame = fill_masked_frame(raw_frame * np.tile(cyan_mask, (int(580/4),int(752/2)))) / 65536.0
magenta_mask = np.array([[0, 0],
                         [1, 0],
                         [0, 0],
                         [0, 1]])
magenta_frame = fill_masked_frame(raw_frame * np.tile(magenta_mask, (int(580/4),int(752/2)))) / 65536.0
#
# Calculateluminance frame
#
y_frame = magenta_frame * 0.05 + cyan_frame * 0.5 + yellow_frame * 0.25 + green_frame * 0.3
u_frame = magenta_frame + cyan_frame - green_frame - yellow_frame
v_frame = magenta_frame + yellow_frame - green_frame - cyan_frame
r_frame = y_frame                    + 1.14  * v_frame
g_frame = y_frame - 0.345  * u_frame - 0.581 * v_frame
b_frame = y_frame + 0.2032 * u_frame
# Reduce image based on median (background) level: reduce_pix = (1 - image_median)/(1 - column_median)*(raw_pix - column_median) + image_median
#scan_med = np.median(fscan)
#for col in range(columns) :
#    col_med = np.median(fscan[:,col])
#    fscan[:,col] -= col_med
#    fscan[:,col] *= (1-scan_med)/(1-col_med)
#    fscan[:,col] += scan_med
#
#if len(argv) < 3 :
#    reduce_scan = 'reduce_scan.fits'
#else :
#    reduce_scan = argv[2]
#fscan_hdu = fits.PrimaryHDU(fscan)
#fscan_hdu.writeto(reduce_scan)

# Plot the result
fig = plt.figure()
#plt.imshow(r_frame)
#fig = plt.figure()
#plt.imshow(g_frame)
#fig = plt.figure()
#plt.imshow(b_frame)
ax = RGBAxes(fig, [0.1, 0.1, 0.8, 0.8])
kwargs = dict(origin="lower", interpolation="nearest")
ax.imshow_rgb(r_frame, g_frame, b_frame, **kwargs)
#ax.RGB.set_xlim(0., 500)
#ax.RGB.set_ylim(0,  500)
plt.show()
