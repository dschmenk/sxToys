all: fits.o

fits.o: src/fits.c fits.h
	$(CC) -I . -c src/fits.c -o fits.o

clean:
	-rm fits.o *~
