all: sxccd.o sxutil.o

sxccd.o: src/sxccd.c sxccd.h
	$(CC) -I . -c src/sxccd.c -o sxccd.o

sxutil.o: src/sxutil.c sxutil.h
	$(CC) -I . -c src/sxutil.c -o sxutil.o

clean:
	-rm sxccd.o sxutil.o *~

install:
	-cp -R rules.d /etc/udev
