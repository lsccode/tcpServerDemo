#CC := arm-linux-gnueabihf-gcc
#LIBEV := libevarm.a
CC := gcc
LIBEV := libev.a
CPPFLAGS += -I ../include

ALL := server
all:$(ALL)

server:server.o
	$(CC) -o server $^ $(LIBEV) -lm

clean:
	rm -rf *.o *.d $(ALL)

