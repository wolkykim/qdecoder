##############################################################
## Makefile for 'qDecoder'                                  ##
##                                                          ##
##    Official distribution : ftp://ftp.nobreak.com         ##
##        Technical contact : nobreak@nobreak.com           ##
##                                                          ##
##                          Developed by 'Seung-young, Kim' ##
##                                                          ##
##                          (c) Nobreak Technologies, Inc.  ##
##############################################################

#
# Define
#
LIBNAME = libqDecoder.a

# Which compiler
CC      = gcc

# System library directory
LIBDIR	= /usr/lib/
HEADERDIR	= /usr/include/

# Where are include files kept
INCLUDE = .

# Options for release
CFLAGS  = -Wall

# Which library archiver
AR	= ar
ARFLAGS = -q

# Which ranlib
RANLIB	= ranlib

OBJ  = $(OBJ1) $(OBJ2)
OBJ1 = qDecoder.o
OBJ2 = 

## Make Library
all: $(OBJ)
	$(AR) $(ARFLAGS) $(LIBNAME) $(OBJ)
	$(RANLIB) $(LIBNAME)

reall: clean all

## Compile Module
%.o:	%.c
	$(CC) -I$(INCLUDE) $(CFLAGS) -c -o $@ $<

## Install Module
install: all
	cp qDecoder.h $(HEADERDIR)/qDecoder.h
	cp $(LIBNAME) $(LIBDIR)/$(LIBNAME)

deinstall:
	rm -f $(HEADERDIR)/qDecoder.h
	rm -f $(LIBDIR)/$(LIBNAME)

## Clear Module
clean:
	rm -f $(OBJ) $(LIBNAME)

