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
LIB     = ../libqDecoder.a

## Main
all:	fetch.cgi multivalue.cgi upload.cgi streamedit.cgi

fetch.cgi:	fetch.o
	$(CC) $(INCLUDE) $(CFLAGS) -o fetch.cgi fetch.o $(LIB)

multivalue.cgi:	multivalue.o
	$(CC) $(INCLUDE) $(CFLAGS) -o multivalue.cgi multivalue.o $(LIB)

upload.cgi:	upload.o
	$(CC) $(INCLUDE) $(CFLAGS) -o upload.cgi upload.o $(LIB)
	chmod 6755 upload.cgi

streamedit.cgi:	streamedit.o
	$(CC) $(INCLUDE) $(CFLAGS) -o streamedit.cgi streamedit.o $(LIB)

## Compile Module
.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

## Clear Module
clean:
	rm -f *.o *.cgi

reall: clean all
