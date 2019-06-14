CC = gcc
CFLAGS = -Wall -g -DDEBUG
OBJECTS = connection.o messages.o commands.o inout.o
LDLIBS = -lreadline
all: master slave
	make clean

master: $(OBJECTS)
slave: $(OBJECTS)

clean:
	rm -rf *.o

purge: clean
	rm -f master slave

remake: purge
	make
	make clean
