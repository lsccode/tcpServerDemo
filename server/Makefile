#CC := arm-linux-gnueabihf-gcc
CC := gcc
CPPFLAGS += -I ../include

ALL := server
all:$(ALL)

server:server.o
	$(CC) -o server $^ libev.a -lm

clean:
	rm -rf *.o *.d $(ALL)

