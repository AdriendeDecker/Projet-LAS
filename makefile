CC=gcc
CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

all : server

server : server server.o utils_v10.o
	$(CC) $(CCFLAGS) -o server server.o utils_v10.o

server.o: server.c utils_v10.h
	$(CC) $(CCFLAGS) -c server.c 

utils_v10.o: utils_v10.c utils_v10.h
	$(CC) $(CCFLAGS) -c utils_v10.c 


clear :
		clear

clean :
	rm -f *.o
	rm -f server