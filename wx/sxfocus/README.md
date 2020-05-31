# sxFocus - Focussing for the Impatient

## Simple Startup

- Plug in camera to USB port
- Launch sxFocus

Real-time images should begin to appear in the Window. Chances are the window will look black at night, or washed out during the day. To quickly get an automatically contrast stretched image, select 'View/Auto Levels' from the menu:

![sxFocus X2](https://github.com/dschmenk/sxToys/blob/master/images/sxfocus-x2.png)

Daytime focussing may require stopping down the telescope lens to achieve an image that isn't completely washed out.

You will notice that all the 'View' menu items have a hot-key. All the options can be driven from the keyboard without modifiers to simplify operation in the dark:

![sxFocus Menu](https://github.com/dschmenk/sxToys/blob/master/images/sxfocus-menu.png)

Instead of selecting 'View/Auto Levels' from the menu, you can simply press the 'A' key. Except for challenging objects to focus on, using the 'Auto Levels' is probably the best option. The default exposure is a quick 10 ms, so increasing the exposure duration (up-arrow key) can help with a dim filled field of stars.

sxFocus uses the on-chip binning and sub-windowing capabilities of the Starlight Xpress cameras to keep the frame rate as high as possible for fast feedback on focus adjustments. 'Zoom In' (right-arrow key) and 'Zoom Out' (left-arrow key). When zooming out to the binning modes, 'X2' and 'X4', the camera sensitivity will increase as well as the frame rate. Very useful for finding gross focus in a hurry. Zoom in to see the changes of fine focus, adjusting the brightness, contrast and exposure as necessary:

![sxFocus 4X](https://github.com/dschmenk/sxToys/blob/master/images/sxfocus-4x.png)

Some other options may be useful such as a red filter over the focus image for dark adapted eyes and a gamma function for teasing out details of star widths.

## Camera Options

If you should start sxFocus before plugging in a camera, or you have multiple cameras connected and want to change which one is being focussed, the 'Camera/Connect Camera..' option will allow you to change which camera is currently attached to the focussing window.

USB 1.1 module users under Linux or MacOS may need to override the camera model should it default to an MX5. For instance, if you have an MX7C connected, the initial focus window may look skewed and show that it is connected to an MX5 (the default for the USB 1.1 module). Simply select the correct camera model from 'Camera/Override Camera...' dialog.

USB 1.1 module users under Windows should follow the process of upgrading the firmware here: https://github.com/dschmenk/sxToys/tree/master/Windows%20Drivers
