CC = mpicc
CFLAGS = -cc=gcc -std=gnu99 -O3
LFLAGS = -lm

EXE = demo

all: 
	$(MAKE) -C actor_framework
	$(MAKE) -C demo_squirrel_solution
	$(CC) $(CFLAGS) -o $(EXE)  actor_framework/*.o demo_squirrel_solution/*.o $(LFLAGS)

.PHONY: clean
clean:
	$(MAKE) clean -C actor_framework
	$(MAKE) clean -C demo_squirrel_solution
	rm -rf demo
