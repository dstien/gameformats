CC = cc
STRIP = strip
CFLAGS = -std=c99 -O2 -Wall -Wextra
BINDIR = /usr/local/bin

OBJS = decdds.o main.o
OUT = decdds

all: $(OUT)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJS)
	$(CC) -o $(OUT) $(OBJS)
	$(STRIP) $(OUT)

install: $(OUT)
	install -m 755 $(OUT) $(BINDIR)

uninstall:
	rm -f $(BINDIR)/$(OUT)

clean:
	rm -f $(OBJS)
	rm -f $(OUT)
