CC := arm-linux-gnueabihf-gcc
#CC := gcc
CPPFLAGS += -I ../include

ALL := server
all:$(ALL)

server:server.o
	$(CC) -o server $^ libevarm.a -lm

clean:
	rm -rf *.o *.d $(ALL)

