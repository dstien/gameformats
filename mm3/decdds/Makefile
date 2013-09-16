CC = cc
STRIP = strip
CFLAGS = -std=c99 -O2 -Wall
BINDIR = /usr/local/bin

IN = decdds.c
OUT = decdds

all $(OUT):
	$(CC) $(CFLAGS) $(IN) -o $(OUT)
	$(STRIP) $(OUT)

install: $(OUT)
	install -m 755 $(OUT) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(OUT)

clean:
	rm -f $(OUT)
