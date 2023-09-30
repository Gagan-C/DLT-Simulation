CC = g++
CFLAGS = -Wall -O2
LIBS = -lcryptopp

all: simulation

simulation: main.cpp
	$(CC) $(CFLAGS) -o simulation main.cpp $(LIBS)

clean:
	rm -f simulation
