OS=$(shell uname)
ifeq ($(OS), Darwin)
else
endif

all:
	$(MAKE) -C sxfocus
	$(MAKE) -C sxtdi
	$(MAKE) -C sxsnap

clean:
	-rm -rf sxToys-MacOS
	$(MAKE) -C sxfocus clean
	$(MAKE) -C sxtdi clean
	$(MAKE) -C sxsnap clean

install:
	$(MAKE) -C sxfocus install
	$(MAKE) -C sxtdi install
	$(MAKE) -C sxsnap install

bundle:
	-rm -rf sxToys-MacOS
	mkdir sxToys-MacOS
	$(MAKE) -C sxfocus bundle
	cp -R sxfocus/sxFocus.app sxToys-MacOS
	$(MAKE) -C sxtdi bundle
	cp -R sxtdi/sxTDI.app sxToys-MacOS
	$(MAKE) -C sxsnap bundle
	cp -R sxsnap/sxSnapShot.app sxToys-MacOS

