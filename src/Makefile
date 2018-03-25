CC = mpicc
CFLAGS = -cc=gcc -std=gnu99 -O3
LFLAGS = -lm

EXE = demo

INC = actor.h pool.h squirrel-functions.h ran2.h customized_actors.h
SRC = actor.c main_actor.c main.c pool.c clock_actor.c landcell_actor.c squirrel_actor.c squirrel-functions.c ran2.c customized_actors.c
OBJ = $(SRC:.c=.o)

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LFLAGS)

$(OBJ): %o: %c $(INC)
	$(CC) $(CFLAGS) -c $< $(LFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJ) $(EXE)