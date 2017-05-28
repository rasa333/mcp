CC             = gcc
CFLAGS         = -O5 -Wunused
LDFLAGS        = -s

%.o : %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $<

objects        		     := $(patsubst %.c,%.o,$(wildcard *.c))


mcp	: $(objects)
	$(CC) $(LDFLAGS) -o mcp $(objects)


$(objects)  : $(wildcard *.h)


clean:
	rm -f *.o mcp

install:
	-mv mcp /usr/local/bin
