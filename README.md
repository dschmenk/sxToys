# sxToys
Astronomy Image Capture Applications for Starlight Xpress (https://www.sxccd.com) cameras.

Using wxWidgets for cross-platform support on Mac, Linux, and Windows. Use more, but smaller, applications for specific purposes (toys). Who doesn't want to explain that they were up late playing with sxToys?

## [SX USB 1.1 Camera Module Upgrade](https://github.com/dschmenk/sxToys/tree/master/Windows%20Drivers)

Upgrade your old Starlight Xpress USB 1.1 camera module, used to provide a USB interface for the old parallel port cameras. Feature parity with original USB 2.0 cameras.

## [sxFocus](https://github.com/dschmenk/sxToys/tree/master/wx/sxfocus): Focussing For The Impatient

sxFocus is used to get good focus quickly. The idea is to have a fast frame rate to quickly get feedback on your focus. The on-camera binning and sub-framing capabilities are used to maximize frame rate.

## [sxTDI](https://github.com/dschmenk/sxToys/tree/master/wx/sxtdi): Time Delay Integration

_It should be noted that common belief was that interline CCDs  (like the Sony CCD in many SX cameras) couldn't use the TDI (drift scanning) method of integration. They were wrong. I figured it out 20 years ago and built it into the firmware. It took awhile for me to get back around to it and provide a generally accessible tool for Time Delay Integration. Unfortunately this feature wasn't able to be tested so it isn't known which cameras retained the TDI functionality_

sxTDI allows Time Delay Integration, or Drift Scanning imaging on many Starlight Xpress cameras. There are some tradeoffs between TDI imaging and regular imaging, but one of the most intriguing aspects is being able to watch the image being scanned in real-time, right on the computer display.

Some of the benefits of Time Delay Integration:

    1. Minimal equipment requirements
  
      - Low end laptop (Windows XP+, Mac OS/X 10.11+, Linux)
    
      - No fancy mount or tripod (no tracking needed)
    
      - Small scope or camera lens
    
    2. Only small patch of unobstructed sky needed
  
    3. Real-time display of image
  
    4. Easy to automate

## Test Image

Here is a sample, unprocessed except for brightness adjustment to bring out the vertical streaking inherent in drift scanning. Using an SXV-H9 camera and Meade 80mm (f/6) refractor (scroll down to check out the M64 and M65 galaxies):

![Test Scan](https://github.com/dschmenk/sxToys/blob/master/images/macscan.jpg).
