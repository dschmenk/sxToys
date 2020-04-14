CXX=`wx-config --cxx`
CC=`wx-config --cxx`
CXXFLAGS=`wx-config --cxxflags` -I ../../libsxccd/src -I ../../fits/src
LDLIBS=`wx-config --libs` -lusb-1.0

sxtdi: sxtdi.o sxccd.o fits.o

clean:
	rm sxtdi *.o

sxccd.o: ../../libsxccd/src/sxccd.c
	$(CC) $(CFLAGS) -c $< -o $@

fits.o: ../../fits/src/fits.c
	$(CC) $(CFLAGS) -c $< -o $@
