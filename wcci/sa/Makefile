CC = gcc 
CFLAGS = -Wall -O2 -fPIC
INCS = -I.
LIBS = -lm
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
BIN = libevaluate.so

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -shared $(CFLAGS) $(INCS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

test: all
	./$(BIN) -f 1 -D 10

clean:
	rm -f *.o $(BIN)
