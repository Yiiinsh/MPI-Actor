CC = mpicc
CFLAGS = -cc=gcc -std=gnu99 -O3
LFLAGS = -lm

INC = ran2.h squirrel-functions.h 
SRC = clock_actor.c landcell_actor.c ran2.c squirrel_actor.c squirrel-functions.c
OBJ = $(SRC:.c=.o)

solution: $(OBJ)

$(OBJ): %o: %c $(INC)
	$(CC) $(CFLAGS) -c $< $(LFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJ)