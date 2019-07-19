CFLAGS = -lm -lcairo -march=native -g

all: penrose

pierwiastki: penrose.c

	gcc penrose.c $(CFALGS) -o penrose

clean:
	rm penrose

.PHONY: all clean
