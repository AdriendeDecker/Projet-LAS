CC = gcc
CFLAGS =-D_DEFAULT_SOURCE -D_XOPEN_SOURCE -D_BSD_SOURCE -std=c11 -pedantic -Wvla -Wall -Werror

all: maint

maint : maint.o utils_v10.o
	$(CC) $(CFLAGS) -o maint maint.o utils_v10.o

maint.o : maint.c utils_v10.h
	$(CC) $(CFLAGS) -c maint.c

utils_v10.o : utils_v10.h utils_v10.c
	$(CC) $(CFLAGS) -c utils_v10.c

clean : 
	rm -f *.o maint