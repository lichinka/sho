CC = gcc 
CFLAGS = `python2-config --cflags`
INCS = `python2-config --includes`
LIBS = `python2-config --ldflags`
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
BIN = python_from_c

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) $(INCS) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

clean:
	rm -f *.o $(BIN)
