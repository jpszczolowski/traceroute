CC = gcc
CFLAGS = -std=c99 -Wall -Wextra

all: traceroute

icmp_tools.o: icmp_tools.h icmp_tools.c
	$(CC) $(CFLAGS) -c icmp_tools.c -o icmp_tools.o

traceroute.o: traceroute.c
	$(CC) $(CFLAGS) -c traceroute.c -o traceroute.o

traceroute: icmp_tools.o traceroute.o
	$(CC) $(CFLAGS) icmp_tools.o traceroute.o -o traceroute

clean:
	rm -f *.o

distclean: clean
	rm -f traceroute