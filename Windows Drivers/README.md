# Upgrading SX USB 1.1 Camera Module

The SX USB 1.1 module was used to provide an USB interface to the original parallel port camera interfaces. Although the original firmware provided by Starlight Xpress was functional, it wasn't compatible with the follow-on integrated USB 2.0 cameras. Luckily, the USB 1.1 module's firmware wasn't written in stone, so to speak. It was downloaded from a file included with the SX install. By replacing this firmware file, the USB 1.1 module's personality can be changed, even to be compatible with the newer cameras. I actually prototyped the USB 2.0 camera's firmware on the 1.1 module. By keeping the USB 1.1 module up-to-date with the development of the USB 2.0 cameras, software could seamlessly work on both cameras. I'm making this firmware available now that there isn't any competition with the USB 2.0 cameras. If you have and old USB 1.1 module, then this can breathe a little life back into your camera.

## Installation
There are two cases to consider: a fresh install and installation over an existing USB 1.1 driver. If you already have the USB 1.1 driver installed, the you can skip this first part.

### SX USB 1.1 Driver Install
Download the USB 1.1 from Starlight Xpress's website here:

https://www.sxccd.com/drivers-downloads

Follow the instructions to install this driver. I had problems with it actually copying all the correct files over, but it won't matter when we use our new firmware file.

Once the initial install is done, you will see an Empty Starlight Xpress device, probably with an error associated with it. Not to worry, just follow the next step.

### Update Firmware File

You will find a list of HEX files, one for each camera supported by the USB 1.1 module. You are going to copy the one that matches your camera to the \\WINDOWS\\SYSTEM32\\DRIVERS directory. You will need to navigate to this directory, probably clicking on the "Show the contents of this directory" along the way. When you get there, you may or may not find a file named '05472131.HEX' already there, depending on the success of the previous install. Rename this file to '05472131.HEX.BAK' and rename the file you just copied over to '05472131.HEX'.

### Preparing for USB 2.0 Drivers

Download the USB 2.0 drivers from Starlight Xpress. Navigate to the directory containing the drivers and .INF files. Copy the 'SXVIO_SX_100.inf' file into this directory. Now unplug the USB 1.1 module and plug it back in. After a couple of beeps, you should get the "New Hardware Found" for an ECHO2 device. Use the advanced method of finding a driver and browse to the USB 2.0 directory. It should be able to load the USB 2.0 drivers for the USB 1.1 module.

### Use
The USB 1.1 module can be used with sxToys and any app that has been written to use the USB 2.0 in a generic manner. Understand that this update comes without any warranty or any support from Starlight Xpress. However, to revert to it's previous functionality, just restore the 05472131.HEX.BAK' to 05472131.HEX'.
