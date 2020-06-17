OS=$(shell uname)
ifeq ($(OS), Darwin)
CXX=`../wxWidgets/build-static/wx-config --cxx`
CC=`../wxWidgets/build-static/wx-config --cxx`
CXXFLAGS=`../wxWidgets/build-static/wx-config --cxxflags` -I ../../libsxccd -I ../../libaip -I ../../fits
LDLIBS=-L../wxWidgets/build-static/lib -framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL \
-lwx_osx_cocoau_xrc-3.0 -lwx_osx_cocoau_qa-3.0 -lwx_baseu_net-3.0 \
-lwx_osx_cocoau_html-3.0 -lwx_osx_cocoau_adv-3.0 -lwx_osx_cocoau_core-3.0 -lwx_baseu_xml-3.0 -lwx_baseu-3.0 \
-framework WebKit -lexpat -lwxregexu-3.0 -lz -lpthread -liconv \
/usr/local/lib/libusb-1.0.a /usr/local/lib/libjpeg.a /usr/local/lib/libtiff.a /usr/local/lib/libpng.a
else
CXX=`wx-config --cxx`
CC=`wx-config --cxx`
CXXFLAGS=`wx-config --cxxflags` -I ../../libsxccd -I ../../libaip -I ../../fits
LDLIBS=`wx-config --libs` -lusb-1.0
endif

sxsnap: sxsnap.o ../../libsxccd/sxccd.o ../../libsxccd/sxutil.o ../../libaip/aip.o ../../fits/fits.o

clean:
	-rm sxsnap *.o
	-rm sxSnapShot.app/Contents/MacOS/sxsnap

ifeq ($(OS), Darwin)
bundle: sxsnap
	-mkdir sxSnapShot.app/Contents/MacOS
	cp sxsnap sxSnapShot.app/Contents/MacOS
else
install: sxsnap
	$(MAKE) -C ../../libsxccd/ install
	cp sxsnap /usr/local/bin
	cp share/sxsnap.desktop /usr/share/applications/
	cp share/sxsnap.png /usr/share/pixmaps
endif

../../libsxccd/sxccd.o: ../../libsxccd/sxccd.h ../../libsxccd/src/sxccd.c
	$(MAKE) -C ../../libsxccd/

../../libsxccd/sxutil.o: ../../libsxccd/sxutil.h ../../libsxccd/src/sxutil.c
	$(MAKE) -C ../../libsxccd/

../../libaip/aip.o: ../../libaip/aip.h ../../libaip/src/aip.c
	$(MAKE) -C ../../libaip/

../../fits/fits.o: ../../fits/fits.h ../../fits/src/fits.c
	$(MAKE) -C ../../fits/
