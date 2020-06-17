# sxSnapShot - Quick and Dirty Snap Shots

## Simple Startup

- Plug in camera to USB port
- Launch sxSnapShot

Enter the number of snap shots to take and exposure duration. Then take them. Review the images by moving forward and backward with 'F' ad 'B'. Delete what you don't want, save the rest.

## Camera Options

If you should start sxSnapShot before plugging in a camera, or you have multiple cameras connected and want to change which one is being focussed, the 'Camera/Connect Camera..' option will allow you to change which camera is currently attached to the focussing window.

USB 1.1 module users under Linux or MacOS may need to override the camera model should it default to an MX5. For instance, if you have an MX7C connected, the initial focus window may look skewed and show that it is connected to an MX5 (the default for the USB 1.1 module). Simply select the correct camera model from 'Camera/Set Camera Model...' dialog.

USB 1.1 module users under Windows should follow the process of upgrading the firmware here: https://github.com/dschmenk/sxToys/tree/master/Windows%20Drivers

## Autonomous

sxSnapShot can be run from the command line or a script to automate taking and saving up up to 100 images.
