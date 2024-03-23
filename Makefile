CC = gcc -g
CFLAGS = -O2 -Wall -Werror -Wextra -std=gnu17

OBJS = traceroute.o receive.o send.o

all: traceroute

traceroute: $(OBJS)
	$(CC) $(CFLAGS) -o traceroute $(OBJS)

receive.o: receive.c receive.h
send.o: send.c send.h
traceroute.o: traceroute.c

clean:
	rm -f *.o

distclean:
	rm -f *.o traceroute
