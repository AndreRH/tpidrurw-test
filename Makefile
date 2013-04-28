LDFLAGS   = -lpthread

all: tpidrurw-test

.PHONY: all clean test

.c.o:
	$(CC) -c $(CFLAGS) $(CEXTRA) -o $@ $<

clean:
	$(RM) main.o tpidrurw-test

tpidrurw-test: main.o
	$(CC) main.o $(LDFLAGS) -o tpidrurw-test

test: tpidrurw-test
	./tpidrurw-test
