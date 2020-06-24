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
    hfill     = vfill[:,0:-1]   + vfill[:,1:]
    return hfill[:-1,:-1]

if len(argv) < 2 :
    print("Usage: ", argv[0], " <RAW FITS IMAGE file> [DARK FITS IMAGE file] [colorized file]")
    exit(0)

raw_frame   = fits.getdata(argv[1])
if len(argv) > 2 :
    raw_frame  -= fits.getdata(argv[2])
green_mask = np.array([[0, 1],
                       [0, 0],
                       [1, 0],
                       [0, 0]])
green_frame = fill_masked_frame(raw_frame * np.tile(green_mask, (int(580/4),int(752/2)))) / 65536.0
yellow_mask = np.array([[0, 0],
                        [0, 1],
                        [0, 0],
                        [0, 1]])
yellow_frame = fill_masked_frame(raw_frame * np.tile(yellow_mask, (int(580/4),int(752/2)))) / 65535.0
cyan_mask = np.array([[0, 0],
                      [1, 0],
                      [0, 0],
                      [1, 0]])
cyan_frame = fill_masked_frame(raw_frame * np.tile(cyan_mask, (int(580/4),int(752/2)))) / 65535.0
magenta_mask = np.array([[1, 0],
                         [0, 0],
                         [0, 1],
                         [0, 0]])
magenta_frame = fill_masked_frame(raw_frame * np.tile(magenta_mask, (int(580/4),int(752/2)))) / 65535.0
#
# Calculate YUV and RGB frames
#
m_coeff = 0.05#0.15
g_coeff = 0.4#0.19
c_coeff = 0.1#0.19
y_coeff = 0.05#0.14
l_coeff = np.array([[m_coeff, g_coeff],
                    [c_coeff, y_coeff],
                    [g_coeff, m_coeff],
                    [c_coeff, y_coeff]])
#y_frame = fill_masked_frame(raw_frame * np.tile(l_coeff, (int(580/4),int(752/2)))) / 65535.0
y_frame = (magenta_frame * 0.05 + cyan_frame * 0.05 + yellow_frame * 0.25 + green_frame * 0.3)
#y_frame = (magenta_frame * 0.1 + cyan_frame * 0.2 + yellow_frame * 0.2 + green_frame * 0.2)
u_frame = (magenta_frame + cyan_frame   - green_frame - yellow_frame) * 0.5
v_frame = (magenta_frame + yellow_frame - green_frame - cyan_frame)   * 0.5
r_frame = y_frame                   + 1.14  * v_frame
g_frame = y_frame - 0.345 * u_frame - 0.581 * v_frame
b_frame = y_frame + 2.032 * u_frame
print ("Max Y = ", np.max(y_frame), "Min Y = ", np.min(y_frame))
print ("Max U = ", np.max(u_frame), "Min U = ", np.min(u_frame))
print ("Max V = ", np.max(v_frame), "Min V = ", np.min(v_frame))
print ("Max R = ", np.max(r_frame), "Min R = ", np.min(r_frame))
print ("Max G = ", np.max(g_frame), "Min G = ", np.min(g_frame))
print ("Max B = ", np.max(b_frame), "Min B = ", np.min(b_frame))
#
# Plot the result
#
#plt.figure()
#plt.imshow(raw_frame, origin="lower")
ax = RGBAxes(plt.figure(), [0.0, 0.0, 1.0, 1.0])
ax.imshow_rgb(r_frame, g_frame, b_frame, origin="lower", interpolation="nearest")
plt.show()
