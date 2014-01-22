#DEBUG = 1

#BINARY = umr.exe
BINARY = umr

CC = gcc
LDFLAGS =
CFLAGS = -Wall -W
ifdef DEBUG
CFLAGS+= -g
else
CFLAGS+= -O2
endif

all: $(BINARY)

UPKG_OBJS = unrealfmt.o
UMR_OBJS = main.o

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BINARY): $(UPKG_OBJS) $(UMR_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f *.o

distclean: clean
	rm -f $(BINARY)

unrealfmt.o: unrealfmtdata.h urf.h umr.h
main.o: umr.h

