CC = mpicc
CFLAGS = -cc=gcc -std=gnu99 -O3
LFLAGS = -lm

INC = actor.h configurations.h customized_actors.h pool.h
SRC = actor.c customized_actors.c main_actor.c main.c pool.c
OBJ = $(SRC:.c=.o)

framework: $(OBJ)

$(OBJ): %o: %c $(INC)
	$(CC) $(CFLAGS) -c $< $(LFLAGS)

.PHONY: clean
clean:
	rm -rf $(OBJ)