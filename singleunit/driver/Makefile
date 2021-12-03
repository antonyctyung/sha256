CC=gcc
CFLAGS=-I.
DEPS = sha256hw.h
OBJ = sha256hw_test.o sha256hw.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)