##############################################################
## Makefile for 'qDecoder'                                  ##
##                                                          ##
##    Official distribution : ftp://ftp.hongik.com          ##
##        Technical contact : nobreak@hongik.com            ##
##                                                          ##
##                          Developed by 'Seung-young, Kim' ##
##                                                          ##
##                               (c) Hongik Internet, Inc.  ##
##############################################################

# Which compiler
CC      = gcc

# Where are include files kept
INCLUDE = -I./ -I../../

# Options for release
CFLAGS  = -Wall

# qDecoder Library Path
QLIB = ../../libqDecoder.a

## Make Library
all:	qFetch

qFetch: $(QLIB) qFetch.o
	$(CC) $(CFLAGS) $(INCLUDE) -o qFetch.cgi qFetch.o $(QLIB)

$(QLIB):
	(cd ../../; make all)

reall: clean all

## Compile Module
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

## Clear Module
clean:
	rm -f *.o qFetch.cgi

