CXX=`wx-config --cxx`
CC=`wx-config --cxx`
CXXFLAGS=`wx-config --cxxflags` -I ../../libsxccd/src
OS=$(shell uname)
LDLIBS=`wx-config --libs` -lusb-1.0
ifeq ($(OS), Darwin)
LDLIBS+= -rpath @executable_path/../Frameworks
endif

sxfocus: sxfocus.o sxccd.o

clean:
	-rm sxfocus *.o
	-rm sxFocus.app/Contents/MacOS/sxfocus

bundle: sxfocus
	-mkdir sxFocus.app/Contents/Frameworks
	-mkdir sxFocus.app/Contents/MacOS
	-cp /usr/local/opt/wxmac/lib/libwx_osx_cocoau_xrc-3.0.dylib sxFocus.app/Contents/Frameworks
	-cp /usr/local/opt/wxmac/lib/libwx_osx_cocoau_webview-3.0.dylib sxFocus.app/Contents/Frameworks
	-cp /usr/local/opt/wxmac/lib/libwx_osx_cocoau_html-3.0.dylib sxFocus.app/Contents/Frameworks
	-cp /usr/local/opt/wxmac/lib/libwx_osx_cocoau_qa-3.0.dylib sxFocus.app/Contents/Frameworks
	-cp /usr/local/opt/wxmac/lib/libwx_osx_cocoau_adv-3.0.dylib sxFocus.app/Contents/Frameworks
	-cp /usr/local/opt/wxmac/lib/libwx_osx_cocoau_core-3.0.dylib sxFocus.app/Contents/Frameworks
	-cp /usr/local/opt/wxmac/lib/libwx_baseu_xml-3.0.dylib sxFocus.app/Contents/Frameworks
	-cp /usr/local/opt/wxmac/lib/libwx_baseu_net-3.0.dylib sxFocus.app/Contents/Frameworks
	-cp /usr/local/opt/wxmac/lib/libwx_baseu-3.0.dylib sxFocus.app/Contents/Frameworks
	-cp /usr/local/opt/libusb/lib/libusb-1.0.0.dylib sxFocus.app/Contents/Frameworks
	cp sxfocus sxFocus.app/Contents/MacOS

sxccd.o: ../../libsxccd/src/sxccd.c
	$(CC) $(CFLAGS) -c $< -o $@
