# SOURCE, EXECUTABLE, INCLUDES, LIBRARY

INCL  = main.h chell.h helpers.h
SRC   = main.c chell.c helpers.c
OBJ   = $(SRC:.c=.o)
LIBS  =
EXE   = main


# COMPILER, LINKER

CC      = gcc
CFLAGS  = -pedantic -Wall -O2
LIBPATH = -L.
LDFLAGS = -o $(EXE) $(LIBPATH) $(LIBS)
CDEBUG  = -pedantic -Wall -g -DDEBUG $(LDFLAGS)
RM      = rm -f


# COMPILE, ASSEMBLE C INTO OBJECT FILES
%.o: %.c
	$(CC) -c $(CFLAGS) $*.c


# LINK OBJ WITH LIBS INTO BINARIES
$(EXE): $(OBJ)
	$(CC) $(LDFLAGS) $(OBJ)


# OBJ DEPEND ON LIBRARIES
$(OBJ): $(INCL)


# RUN COMPILED PROGRAM
run: $(EXE)
	./main


# CREATE GDB CAPABLE EXE WITH DEBUG FLAGS
debug:
	$(CC) $(CDEBUG) $(SRC)


# CLEAN UP
clean:
	$(RM) $(OBJ) $(EXE) core a.out
