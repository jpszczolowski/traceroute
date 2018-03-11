CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

all: traceroute

icmp_tools.o: icmp_tools.h icmp_tools.c
traceroute.o: traceroute.c
traceroute: icmp_tools.o traceroute.o

clean:
	rm -f *.o

distclean: clean
	rm -f traceroute