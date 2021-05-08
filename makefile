CC=gcc
CCFLAGS=-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

all : maint stat

server : server server.o utils_v10.o
	$(CC) $(CCFLAGS) -o server server.o utils_v10.o

server.o: server.c utils_v10.h
	$(CC) $(CCFLAGS) -c server.c 

maint : maint.o utils_v10.o
	$(CC) $(CFLAGS) -o maint maint.o utils_v10.o

maint.o : maint.c utils_v10.h
	$(CC) $(CFLAGS) -c maint.c

stat : stat.o utils_v10.o
	$(CC) $(CFLAGS) -o stat stat.o utils_v10.o

stat.o : stat.c utils_v10.h
	$(CC) $(CFLAGS) -c stat.c

utils_v10.o: utils_v10.c utils_v10.h
	$(CC) $(CCFLAGS) -c utils_v10.c 


clear :
		clear

clean :
	rm -f *.o
	rm -f server
	rm -f maint