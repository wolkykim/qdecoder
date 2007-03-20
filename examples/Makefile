##############################################################
## Makefile for 'qDecoder Samples'                          ##
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
INCLUDE = -I../

# Options for release
CFLAGS  = -Wall

# qDecoder Library Path
QLIB = ../libqDecoder.a

## Main
all:	fetch.cgi multivalue.cgi upload.cgi streamedit.cgi

fetch.cgi: $(QLIB) fetch.o
	$(CC) $(INCLUDE) $(CFLAGS) -o fetch.cgi fetch.o $(QLIB)

multivalue.cgi:	$(QLIB) multivalue.o
	$(CC) $(INCLUDE) $(CFLAGS) -o multivalue.cgi multivalue.o $(QLIB)

upload.cgi: $(QLIB) upload.o
	$(CC) $(INCLUDE) $(CFLAGS) -o upload.cgi upload.o $(QLIB)
	chmod 6755 upload.cgi

streamedit.cgi: $(QLIB) streamedit.o
	$(CC) $(INCLUDE) $(CFLAGS) -o streamedit.cgi streamedit.o $(QLIB)

$(QLIB):
	(cd ../; make all)

## Compile Module
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

## Clear Module
clean:
	rm -f *.o *.cgi

reall: clean all
