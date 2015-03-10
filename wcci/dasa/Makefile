CC = gcc 
CFLAGS = -fno-strict-aliasing -march=x86-64 -mtune=generic -O2 -pipe
INCS = `python2-config --includes`
LIBS = `python2-config --ldflags`
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
BIN = dasa

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

test: all
	./$(BIN) -f 1 -D 10

clean:
	rm -f *.o $(BIN)
