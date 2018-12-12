CC := gcc
CPPFLAGS += -I ../include

ALL := client
all:$(ALL)

client:client.o
	$(CC) -o client $^ libev.a -lm

clean:
	rm -rf *.o *.d $(ALL)
