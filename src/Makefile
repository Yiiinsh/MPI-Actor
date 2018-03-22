CC = mpicc
CFLAGS = -cc=gcc -std=c99 -O3

EXE = demo

INC = actor.h main_actor.h msg_tag.h pool.h clock_actor.h landcell_actor.h
SRC = actor.c main_actor.c main.c pool.c clock_actor.c landcell_actor.c
OBJ = $(SRC:.c=.o)

all : $(EXE)

$(EXE) : $(OBJ)
    $(CC) $(CFLAGS) -o $@ $^

$(OBJ) : %o : %c $(INC)
    $(CC) $(CFLAGS) -c $<

.PHONY: clean
clean :
    rm -rf $(OBJ) $(EXE)