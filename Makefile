# Standard makefile (Andyw style!)

#PROFILE = 1
#DEBUG = 1

EXECNAME = umr
GLOBALDEPS = Makefile umr.h
OBJS = main.o unrealfmt.o unrealfmtdata.o

CC = gcc
CFLAGS = -Wall

ifdef DEBUG
CFLAGS += -g
else
ifdef PROFILE
CFLAGS += -p
else
CFLAGS += -O2
endif
endif

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(EXECNAME) $(OBJS)
    
clean:
	@rm -f *.o *~

realclean: clean
	@rm -f $(EXECNAME)


main.o: $(GLOBALDEPS)
unrealfmt.o: $(GLOBALDEPS) urf.h
unrealfmtdata.o: $(GLOBALDEPS) urf.h
