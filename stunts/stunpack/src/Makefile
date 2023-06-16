ifdef WIN32
	CC = i586-mingw32msvc-cc
	STRIP = i586-mingw32msvc-strip
	OUT = stunpack.exe
else
ifdef DOS
	CC = wcl386
	OUT = stunpack.exe
else
	CC = cc
	STRIP = strip
	OUT = stunpack
endif
endif

ifdef DOS
LFLAGS = -l=dos4g
CFLAGS = $(LFLAGS) -c -l=dos4g -fo=$@
LNOUT = -fe=$(OUT)
CCOUT = 
else
LFLAGS = -O2 -Wall
CFLAGS = $(LFLAGS) -c
BINDIR = /usr/local/bin
LNOUT = -o $(OUT)
CCOUT = -o $@
endif

SRCS = stunpack.c main.c
OBJS = $(SRCS:.c=.o)

all $(OUT): $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) $(LNOUT)
ifndef DOS
	$(STRIP) $(OUT)
endif

%.o: %.c
	$(CC) $(CFLAGS) $< $(CCOUT)

install: $(OUT)
ifndef DOS
	install -m 755 $(OUT) $(BINDIR)
endif

uninstall:
ifndef DOS
	rm -f $(BINDIR)/$(OUT)
endif
 
clean:
	rm -f *~ *.o stunpack stunpack.exe
