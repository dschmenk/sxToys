## Building for Windows:

Download [wxWidgets Windows Installer](https://www.wxwidgets.org/downloads/)

Copy here and rename `wxWidgets`

    cd wxWidgets\build\msw
    
    nmake /f makefile.vc BUILD=release SHARED=1
    
    cd ..\..\..

Download [CFITSIO Library](http://heasarc.gsfc.nasa.gov/FTP/software/fitsio/c/cfitsio_latest.tar.gz)

    ren cfitsio-3.XX cfitsio
    
    cd cfitsio
    
    mkdir build
    
    cd build
    
    cmake.exe -G "NMake Makefiles" ..
    
    cmake.exe --build . --config Release
    
    cd ..\..
    
Build toys

    nmake /f makefile.vc

    copy /s sxToys-Win32 "C:\Program Files\sxToys"

Create links to sxFocus and sxTDI in Start menu

## Building for MacOS/OSX:

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
    
Build CFITSIO library

    ln -s cfitsio-3.48 cfitsio
    
    cd cfitsio
    
    ./configure 

    make bundle

## Building for Linux

Install development packages for `libwxgtk3.0-dev`

Install development packages for `libusb-1.0-0-dev`

Install development package for `libcfitsio3-dev`

    make

    sudo make install
