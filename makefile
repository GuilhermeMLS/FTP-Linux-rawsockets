CC = gcc
CFLAGS = -Wall -g -DDEBUG
OBJECTS = conexaoRawSocket.o messages.o commands.o inout.o
LDLIBS = -lreadline
all: master slave kermit
	make clean

master: $(OBJECTS)
kermit: $(OBJECTS)
slave: $(OBJECTS)

clean:
	rm -rf *.o

purge: clean
	rm -f master slave kermit

remake: purge
	make
	make clean
