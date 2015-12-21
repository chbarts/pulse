CC=gcc
CFLAGS=-O3 -march=native -flto -lm `pkg-config --libs --cflags libpulse-simple`
DEPS=handle_ferr.h
OBJ=dtmf.o pacat-simple.o tone.o handle_ferr.o

all: dtmf tone pacat-simple

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

dtmf: dtmf.o handle_ferr.o
	$(CC) dtmf.o handle_ferr.o -o dtmf $(CFLAGS)

tone: tone.o handle_ferr.o
	$(CC) tone.o handle_ferr.o -o tone $(CFLAGS)

pacat-simple: pacat-simple.o handle_ferr.o
	$(CC) pacat-simple.o handle_ferr.o -o pacat-simple $(CFLAGS)

clean:
	rm dtmf tone pacat-simple $(OBJ)
