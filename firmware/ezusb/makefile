all: ../sx_ezusb_code.h mx5.hex mx5c.hex mx7.hex mx7c.hex mx9.hex hx5.hex hx9.hex

clean:
	-rm *.asm
	-rm *.lst
	-rm *.lnk
	-rm *.map
	-rm *.rel
	-rm *.rst
	-rm *.sym
	-rm *.ihx
	-rm *.hex
	-rm hex2h

hex2h : hex2h.c
	gcc hex2h.c -o hex2h

# Build static camera model versions of the HEX file (for Windows).
mx5.hex: sx_ezusb.c
	sdcc -D CAMERA_MODEL=0x45 -D NUM_SERIAL_PORTS=0 sx_ezusb.c
	packihx < sx_ezusb.ihx > mx5.hex

mx5c.hex: sx_ezusb.c
	sdcc -D CAMERA_MODEL=0xC5 -D NUM_SERIAL_PORTS=0 sx_ezusb.c
	packihx < sx_ezusb.ihx > mx5c.hex

mx7.hex: sx_ezusb.c
	sdcc -D CAMERA_MODEL=0x47 -D NUM_SERIAL_PORTS=0 sx_ezusb.c
	packihx < sx_ezusb.ihx > mx7.hex

mx7c.hex: sx_ezusb.c
	sdcc -D CAMERA_MODEL=0xC7 -D NUM_SERIAL_PORTS=0 sx_ezusb.c
	packihx < sx_ezusb.ihx > mx7c.hex

mx9.hex: sx_ezusb.c
	sdcc -D CAMERA_MODEL=0x49 -D NUM_SERIAL_PORTS=0 sx_ezusb.c
	packihx < sx_ezusb.ihx > mx9.hex

hx5.hex: sx_ezusb.c
	sdcc -D CAMERA_MODEL=0x05 -D NUM_SERIAL_PORTS=0 sx_ezusb.c
	packihx < sx_ezusb.ihx > hx5.hex

hx9.hex: sx_ezusb.c
	sdcc -D CAMERA_MODEL=0x09 -D NUM_SERIAL_PORTS=0 sx_ezusb.c
	packihx < sx_ezusb.ihx > hx9.hex

05472131.hex: sx_ezusb.c
	sdcc  -D NUM_SERIAL_PORTS=0 sx_ezusb.c
	packihx < sx_ezusb.ihx > 05472131.hex

sx_ezusb.hex: sx_ezusb.c
	sdcc sx_ezusb.c
	packihx < sx_ezusb.ihx > sx_ezusb.hex

../sx_ezusb_code.h : sx_ezusb.hex hex2h
	./hex2h < sx_ezusb.hex > ../sx_ezusb_code.h

