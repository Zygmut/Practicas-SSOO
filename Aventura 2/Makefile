##################################################################
###                Alberto Cugat Martín                        ###
###                Jaume Julià Vallespir                       ### 
###                Rubén Palmer Pérez                          ###
##################################################################
CC=gcc
CFLAGS=-c -g -Wall -std=c99
#LDFLAGS=
 
SOURCES=nivel1.c nivel2.c nivel3.c nivel4.c nivel5.c nivel6.c my_shell.c
LIBRARIES= #.o 
INCLUDES= #.h
PROGRAMS=nivel1 nivel2 nivel3 nivel4 nivel5 nivel6 my_shell
OBJS=$(SOURCES:.c=.o)
 
all: $(OBJS) $(PROGRAMS)
 
$(PROGRAMS): $(LIBRARIES) $(INCLUDES)
	$(CC) $@.o -o $@ $(LDFLAGS) $(LIBRARIES)

%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -o $@ -c $<
 
.PHONY: clean
clean:
	rm -rf *.o *~ *.tmp $(PROGRAMS)
