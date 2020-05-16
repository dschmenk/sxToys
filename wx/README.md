Building for MacOS/OSX:

  Download wxWidgets here.
  
  Create symlink for `wxWidgets` to current version:
  
  `ln -s wxWidgets-3.0.xx wxWidgets`
  
  Build static library version of wxWidgets:
  
  `cd wxWidgets`
  
  `mkdir build-static`
  
  `cd build-static`
  
  Select which min version to build to. Must have the appropriate SDK installed:
  
  `../configure --with-macosx-version-min=10.13  --enable-static=yes`
  
  Build sxToys:
  
  `cd ../../sxfocus`
  
  `make bundle`
  
  `cd ../sxtdi`
  
  `make bundle`
  
