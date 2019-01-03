GTKINCS = $(shell pkg-config --cflags gtk+-3.0)
GTKLIBS = $(shell pkg-config --libs gtk+-3.0)
PROG = main
OBJS = main.o

all: $(PROG)

clean:
	rm -f $(PROG) *.o

$(PROG): $(OBJS)
	$(CC) $(OBJS) $(GTKLIBS) -o $(PROG)

%.o: %.c
	$(CC) -c $(CFLAGS) $(GTKINCS) $< -o $@
