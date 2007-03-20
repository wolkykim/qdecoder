########################################
##
## qDecoder Makefile
##
## Designed by Seung-young, Kim
##
## [Hongik Shinan Network Security]
##
########################################

##
## Define
##

CC      = gcc -Wall        # For GNU C Compiler(gcc)
#CC      = cc               # For the other C Compiler 

OBJ  = $(OBJ1) $(OBJ2)
OBJ1 = qDecoder.o
OBJ2 = 

##
## Main
##
all:	$(OBJ)

## Link Module

## Compile Module
%.o:	%.c
	$(CC) -c $<

## Clear Module
clean:
	rm -f $(OBJ) 

