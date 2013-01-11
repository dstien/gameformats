ifdef WIN32
	CC = i586-mingw32msvc-cc
	STRIP = i586-mingw32msvc-strip
	OUT = stunpack.exe
else
	CC = cc
	STRIP = strip
	OUT = stunpack
endif

LFLAGS = -O2 -Wall
CFLAGS = $(LFLAGS) -c
BINDIR = /usr/local/bin

SRCS = stunpack.c main.c
OBJS = $(SRCS:.c=.o)

all $(OUT): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o $(OUT)
	$(STRIP) $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

install: $(OUT)
	install -m 755 $(OUT) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(OUT)
 
clean:
	rm -f *~ *.o stunpack stunpack.exe
