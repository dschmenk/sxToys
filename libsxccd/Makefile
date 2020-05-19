all: sxccd.o

sxccd.o: src/sxccd.c
	$(CC) -c src/sxccd.c -o sxccd.o

clean:
	-rm sxccd.o *~

install:
	-cp -R rules.d /etc/udev
