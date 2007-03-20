########################################
##
## qDecoder Makefile
##
## Designed by 'Seung-young, Kim'
##
## (c) Nobreak Technologies
##
########################################

##
## Define
##

CC      = gcc -Wall        # For GNU C Compiler(gcc)
#CC      = cc               # For the other C Compiler 

AR	= ar
RANLIB	= ranlib
LIBNAME	= qDecoder.a

OBJ  = $(OBJ1) $(OBJ2)
OBJ1 = qDecoder.o
OBJ2 = 

## Make Library
all: $(OBJ)
	$(AR) q $(LIBNAME) $(OBJ)
	$(RANLIB) $(LIBNAME)
reall: clean all

## Compile Module
%.o:	%.c
	$(CC) -c $<

## Clear Module
clean:
	rm -f $(OBJ) $(LIBNAME)

