CC             = gcc
CFLAGS         = -O -Wunused
LDFLAGS        = -s

%.o : %.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $<

objects        		     := $(patsubst %.c,%.o,$(wildcard *.c))


all mcp	: $(objects)
	$(CC) $(LDFLAGS) -o mcp $(objects)


$(objects)  : $(wildcard *.h)


clean:
	rm -f *.o mcp

install:
	-mv mcp /usr/local/bin
