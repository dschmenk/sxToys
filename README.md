# sxToys
Astronomy Image Capture Applications for Starlight Xpress (https://www.sxccd.com) cameras.

Using wxWidgets for cross-platform support on Mac, Linux, and Windows. Use more, but smaller, applications for specific purposes (toys). Who doesn't want to explain that they were up late playing with sxToys?

## [SX USB 1.1 Camera Module Upgrade](https://github.com/dschmenk/sxToys/tree/master/Windows%20Drivers)

Upgrade your old Starlight Xpress USB 1.1 camera module, used to provide a USB interface for the old parallel port cameras. Feature parity with original USB 2.0 cameras.

## [sxFocus](https://github.com/dschmenk/sxToys/tree/master/wx/sxfocus): Focussing For The Impatient

sxFocus is used to get good focus quickly. The idea is to have a fast frame rate to quickly get feedback on your focus. The on-camera binning and sub-framing capabilities are used to maximize frame rate.

Look at the menu options for adjusting zoom and exposure times. The “Automatic Adjust” will scale the brightness to the current image. The status bar show the maximum displayed pixel value. Higher equals better focus. I would suggest getting coarse focus during daylight on terrestrial targets and fine tuning at night. If the image is washed out at 65535, try zooming in (2X binning will look washed out in any kind of light) or stopping down the lens. If Auto only gives you a noisy image, increase the exposure.

All of the menu options can be driven directly from the keyboard.

## [sxTDI](https://github.com/dschmenk/sxToys/tree/master/wx/sxtdi): Time Delay Integration

_It should be noted that common belief was that interline CCDs  (like the Sony CCD in many SX cameras) couldn't use the TDI (drift scanning) method of integration. They were wrong. I figured it out 20 years ago and built it into the firmware. It took awhile for me to get back around to it and provide a generally accessible tool for Time Delay Integration._

Once you have a good focus, sxTDI has a mode to help align the camera with the star drift. If you thought polar alignment was a pain, you haven’t seen nothin’ yet. Here, the idea is to align the camera with the drift of the stars. The bottom of the camera will point towards the direction stars are coming from. To orient you, the “Align” menu option will super-impose continuous images. You want the stars to start from the left, follow the green guide lines, and exit the right. The best star will be tracked and displayed on the status bar - enlarge the window if the values are truncated. You want the first coordinate to remain as constant as possible (within 1 pixel). If it drifts up or down, you have to rotate the camera to get the star trails exactly horizontal. This will frustrate you, but will pay off with tight star images later. Stop and restart the alignment after every adjustment, or just press 'A' to automatically restart.

Once the camera is aligned and the first coordinate stays within a pixel, watch the rows/sec rate number. Once it stabilizes, you can stop the alignment and move to scanning. sxTDI will display the scan in real time. You can’t do that with normal imaging! It is very mesmerizing. Go ahead and start a scan. The image will fill in from the right. The first full frame is a throw-away - the exposure is ramping up. Once the image fills to the left edge, the image will now shift to the right as new rows are read in from the camera. You should see nice point star images.

If you see horizontal streaks, this means one of two things: the scan rate wasn’t measured correctly or the computer can’t keep up with reading the camera. If your rate measurement had stabilized in the align step, then most likely your computer can’t keep up. The image display is very CPU intensive, so again you have two options: increase Y binning or minimize the window. Y binning will halve or quarter the resolution in the scan direction. It will increase the camera sensitivity as well as reduce the workload the computer is doing to display the image. You can also minimize the window, but you won’t be able to watch the scan progress.

If you see vertical streaks instead of point stars, you aren't aligned along the star trails. With wide angles, it may not be possible to completely remove the difference in star tracking from top to bottom.

If you see crescents or diagonals, then you have a combination of the above two.

If you see bright stars streak and bloom, you now know what it's like to have a CCD chip without anti-blooming. While it is true the Sony CCDs have circuitry to keep blooming down when moving a single image frame to the interline transfer registers, it doesn't help the interline registers from getting overloaded when continuously transferring charge for every row of pixels in the CCD. You can try stopping down the lens to reduce the bloom effect but also reduce sensitivity. Y binning can be a compromise between the two. Experimentation is half the fun.

If you see the frame wash out during the first frame, there is probably too much sky glow. Either wait until the sky darkens or stop the lens down.

You can stop the scan at anytime without losing the data collected. Just save the image before starting a new scan. Otherwise, let it run and come back later (probably the morning). It’s a bit like Christmas morning looking at the result. How many galaxies can you spot? I use [GIMP](https://gimp.org), the Gnu Image Manipulation Program, which has greatly improved in the last few years to deal with higher bit depth images and can read FITS files directly.

Make sure you adjust your computer's power settings so it doesn't go to sleep during scanning.

It is possible to drive all the parameters from the command line so that the drift scan can be automated.

## Useful Links

[Time Delay Integration Presentation](https://nexsci.caltech.edu/workshop/2005/presentations/Rabinowitz.pdf)

[Drift Scan Technique (Audine Camera)](http://www.astrosurf.com/audine/English/result/scan.htm)

[Drift Scan Imaging (SBIG Camera)](http://www.company7.com/library/sbig/pdffiles/drftscan.pdf)

## Test Image

Here is a sample (first ~45 minutes of 6 hour scan) using an SXV-H9 camera and Meade 80mm (f/6) refractor (scroll down):

![Test Scan](https://github.com/dschmenk/sxToys/blob/master/images/scopescan1.jpg).
