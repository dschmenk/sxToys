# sxTDI - Time Delay Integration or Drift Scanning

## Camera Setup

There are many ways to mount your camera for effective TDI drift scanning. Here are two options I use: an equatorial mount with short focal length scope and a simple camera tripod with commodity SLR lens:

![sxTDI Mount Options](https://github.com/dschmenk/sxToys/blob/master/images/sxtdi-mounts.jpg)

The mount doesn't have to track during TDI scanning, so you have many low cost options.

### Simple Lens+Tripod

If you want to try TDI scanning without much investment, or your telescope and mount are busy, here is a possible option:

![sxTDI Camera Lens Mount](https://github.com/dschmenk/sxToys/blob/master/images/sxtdi-lensmount.jpg)

My cheap solution circled here involves a bed frame clamp (Google: "bed frame rail clamp"), available at just about any hardware store, and some ZIP ties to securely connect the camera to a typical photo tripod. In an odd twist of fate, the bed frame clamp is threaded the same as a standard camera body for an easy tripod attachment. You want the ZIP ties as tight as you can get them (I use pliers to pull on them). The smooth barrel of the camera extension will still be able to rotate with just a little effort, but not inadvertently slip while imaging.

## Startup

- Plug in camera to USB port
- Launch sxTDI

## Alignment

The easiest way to get gross alignment is to watch the direction of the star drift during focussing. You want the camera oriented such that the stars drift towards the top of the window. Honest.

![sxFocus Drift](https://github.com/dschmenk/sxToys/blob/master/images/sxtdi-sxfocusdrift.png)

For fine alignment and scan rate measurement, select the 'Scan/Align' menu item. Just like sxFocus, many of the operations can be selected with a simple key press, 'A' in this case. You will see the window clear to black with horizontal green alignment lines. This image is actually rotated by 90 degrees so the the stars should drift left to right. The window will superimpose every frame read from the camera to show the overall drift of the stars:

![sxTDI Align](https://github.com/dschmenk/sxToys/blob/master/images/sxtdi-align.png)

sxTDI will pick a "best candidate" star to track. It will calculate the up and down drift as well as the scan rate and display them in the status bar. If the status bar is too small, simply resize the window until you can see it fully. Should the tracking star get occluded by clouds or run off the frame, you will see this windows pop up, and alignment will cease:

![sxTDI Lost](https://github.com/dschmenk/sxToys/blob/master/images/sxtdi-lost.png)

If the tracking coordinate increases, then the camera needs to be rotated clockwise. Note the orientation of the camera - it is pointing West-ish.

![sxTDI Drift Up](https://github.com/dschmenk/sxToys/blob/master/images/sxtdi-clockwise.jpg)

Conversely, if the tracking coordinate decreases, the camera needs to be rotated counter-clockwise:

![sxTDI Drift Down](https://github.com/dschmenk/sxToys/blob/master/images/sxtdi-counterclock.jpg)

During alignment, there is no need to stop and start the alignment. Simply press 'A' and the alignment will restart. However, you will often see the tracking star lost dialog when you make adjustments. Simply click 'Okay' or press the 'Return' key to dismiss and 'A' to resume alignment.

As the alignment gets closer and closer, the amount of adjustment decreases. Until barely any rotation is made to keep the tracking star within 1 pixel. It will be frustrating, but the results it worth it. Once aligned, the camera and mount don't have to be adjusted. The scan rate should quickly converge once aligned. Take note of the scan rate. You can just enter it in on subsequent scans without having to re-align (unless the camera is moved) through the 'Scan/Scan Rate...' dialog.

## TDI Scanning

To initiate the TDI scanning, select 'Scan/TDI' or press 'T'. sxTDI will display the scan in real time. You can’t do that with normal imaging! It is very mesmerizing. The image will fill in from the right. The first full frame is a throw-away - the exposure is ramping up.

![sxTDI Ramp](https://github.com/dschmenk/sxToys/blob/master/images/sxtdi-scanramp.png)

Once the image fills to the left edge, the image will now shift to the right as new rows are read in from the camera. You should see nice point star images. If you see horizontal streaks, this means one of two things: the scan rate wasn’t measured correctly or the computer can’t keep up with reading the camera. If your rate measurement had stabilized in the align step, then most likely your computer can’t keep up. The image display is very CPU intensive, so again you have two options: increase Y binning or minimize the window. Y binning will halve or quarter the resolution in the scan direction. It will increase the camera sensitivity as well as reduce the workload the computer is doing to display the image. You can also minimize the window, but you won’t be able to watch the scan progress.

If you see vertical streaks instead of point stars, you aren't aligned along the star trails. With wide angles, it may not be possible to completely remove the difference in star tracking from top to bottom.

If you see crescents or diagonals, then you have a combination of the above two.

If you see bright stars streak and bloom, you now know what it's like to have a CCD chip without anti-blooming. While it is true the Sony CCDs have circuitry to keep blooming down when moving a single image frame to the interline transfer registers, it doesn't help the interline registers from getting overloaded when continuously transferring charge for every row of pixels in the CCD. You can try stopping down the lens to reduce the bloom effect but also reduce sensitivity. Y binning can be a compromise between the two. Experimentation is half the fun.

If you see the frame wash out during the first frame, there is probably too much sky glow. Either wait until the sky darkens or stop the lens down.

![sxTDI Ramp](https://github.com/dschmenk/sxToys/blob/master/images/sxtdi-scanning.png)

Make sure you adjust your computer's power settings so it doesn't go to sleep during scanning. The Mac is a bit of a challenge getting it to stay awake for the duration of the scan. Try a program like [Amphetamine](https://apps.apple.com/us/app/amphetamine/id937984704?mt=12), available for free in the Mac App Store.

## Camera Options

If you should start sxTDI before plugging in a camera, or you have multiple cameras connected and want to change which one is being scanned, the 'Camera/Connect Camera..' option will allow you to change which camera is currently attached to the scanning window.

USB 1.1 module users under Linux or MacOS may need to override the camera model should it default to an MX5. For instance, if you have an MX7C connected, the initial focus window may look skewed and show that it is connected to an MX5 (the default for the USB 1.1 module). Simply select the correct camera model from 'Camera/Override Camera...' dialog.

USB 1.1 module users under Windows should follow the process of upgrading the firmware here: https://github.com/dschmenk/sxToys/tree/master/Windows%20Drivers

## Saving The Image

sxTDI will save the image to a FITS file. You can stop the scan at anytime without losing the data collected. Just save the image before starting a new scan. Otherwise, let it run and come back later (probably the morning). It’s a bit like Christmas morning looking at the result. How many galaxies can you spot? I use [GIMP](https://gimp.org), the Gnu Image Manipulation Program, which has greatly improved in the last few years to deal with higher bit depth images and can read FITS files directly.

## Useful Links

[Time Delay Integration Presentation](https://nexsci.caltech.edu/workshop/2005/presentations/Rabinowitz.pdf)

[Drift Scan Technique (Audine Camera)](http://www.astrosurf.com/audine/English/result/scan.htm)

[Drift Scan Imaging (SBIG Camera)](http://www.company7.com/library/sbig/pdffiles/drftscan.pdf)
