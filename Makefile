CFLAGS = -lm -lcairo -march=native -g

all: penrose

penrose: penrose.c
	gcc penrose.c $(CFLAGS) -o penrose

clean:
	rm penrose

.PHONY: all clean
