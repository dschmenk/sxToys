CXX=`wx-config --cxx`
CC=`wx-config --cxx`
CXXFLAGS=`wx-config --cxxflags` -I ../../libsxccd/src
LDLIBS=`wx-config --libs` -lusb-1.0

sxfocus: sxfocus.o sxccd.o

clean:
	rm sxfocus *.o

sxccd.o: ../../libsxccd/src/sxccd.c
	$(CC) $(CFLAGS) -c $< -o $@
