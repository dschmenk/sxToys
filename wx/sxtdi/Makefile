OS=$(shell uname)
ifeq ($(OS), Darwin)
CXX=`../wxWidgets/build-static/wx-config --cxx`
CC=`../wxWidgets/build-static/wx-config --cxx`
CXXFLAGS=`../wxWidgets/build-static/wx-config --cxxflags` -I ../../libsxccd -I ../../libaip -I ../../fits/src
LDLIBS=-L../wxWidgets/build-static/lib -framework IOKit -framework Carbon -framework Cocoa -framework AudioToolbox -framework System -framework OpenGL \
-lwx_osx_cocoau_xrc-3.0 -lwx_osx_cocoau_qa-3.0 -lwx_baseu_net-3.0 \
-lwx_osx_cocoau_html-3.0 -lwx_osx_cocoau_adv-3.0 -lwx_osx_cocoau_core-3.0 -lwx_baseu_xml-3.0 -lwx_baseu-3.0 \
-framework WebKit -lexpat -lwxregexu-3.0 -lz -lpthread -liconv \
/usr/local/lib/libusb-1.0.a /usr/local/lib/libjpeg.a /usr/local/lib/libtiff.a /usr/local/lib/libpng.a
else
CXX=`wx-config --cxx`
CC=`wx-config --cxx`
CXXFLAGS=`wx-config --cxxflags` -I ../../libsxccd -I ../../libaip -I ../../fits/src
LDLIBS=`wx-config --libs` -lusb-1.0
endif

sxtdi: sxtdi.o ../../libsxccd/sxccd.o ../../libsxccd/sxutil.o ../../libaip/aip.o fits.o

clean:
	-rm sxtdi *.o
	-rm sxTDI.app/Contents/MacOS/sxtdi

install: sxtdi
	$(MAKE) -C ../../libsxccd/ install
	cp sxtdi /usr/local/bin
	cp share/sxtdi.desktop /usr/share/applications/
	cp share/sxtdi.png /usr/share/pixmaps

bundle: sxtdi
	-mkdir sxTDI.app/Contents/MacOS
	cp sxtdi sxTDI.app/Contents/MacOS

../../libsxccd/sxccd.o: ../../libsxccd/sxccd.h ../../libsxccd/src/sxccd.c
	$(MAKE) -C ../../libsxccd/

../../libsxccd/sxutil.o: ../../libsxccd/sxutil.h ../../libsxccd/src/sxutil.c
	$(MAKE) -C ../../libsxccd/

../../libaip/aip.o: ../../libaip/aip.h ../../libaip/src/aip.c
	$(MAKE) -C ../../libaip/

fits.o: ../../fits/src/fits.c
	$(CC) $(CFLAGS) -c $< -o $@
