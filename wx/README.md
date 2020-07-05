Note: I've backed out the CFITSIO library. Just too big and too many changes just to write simple little FITS images. If I need to read FITS images, I will return to CFITSIO for those operations.

## Building for Windows:

~With the addition of CFITSIO, the minimum Windows version is going to Windows 7 32-bit (maybe Vista).~

Download [wxWidgets Windows Installer](https://www.wxwidgets.org/downloads/)

Copy here and rename `wxWidgets`

    cd wxWidgets\build\msw
    
    nmake /f makefile.vc BUILD=release SHARED=1 MONOLITHIC=1 VENDOR=sxwx
    
    cd ..\..\..

~Download [CFITSIO Library](http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio_latest.tar.gz) to ..~

~cd ..~

~ren cfitsio-3.XX cfitsio~
    
~cd ../cfitsio~
    
~mkdir build~
    
~cd build~
    
~cmake.exe -G "NMake Makefiles" ..~
    
~cmake.exe --build . --config Release~

~cd ..\..\wx~
    
Build toys

    nmake /f makefile.vc

    copy /s sxToys-Win32 "C:\Program Files\sxToys"

Create links to sxFocus and sxTDI in Start menu

## Building for MacOS/OSX:

From homebrew install: libusb libpng libjpeg libtiff
    
Download wxWidgets here.

Create symlink for `wxWidgets` to current version:

    ln -s wxWidgets-3.0.xx wxWidgets

Build static library version of wxWidgets:

    cd wxWidgets

    mkdir build-static

    cd build-static

Select which min version to build to. Must have the appropriate SDK installed:

    ../configure --with-macosx-version-min=10.11  --disable-shared

    make

    cd ../..
    
~Build CFITSIO library to ..~

~cd ..~
    
~ln -s cfitsio-3.48 cfitsio~
    
~cd cfitsio~
    
~./configure~
    
~make~
    
~cd ../wx~
    
Build toys
    
    make bundle

## Building for Linux

Install development package `libwxgtk3.0-dev`

Install development package `libusb-1.0-0-dev`

~Install development package `libcfitsio-dev`~

    make

    sudo make install
