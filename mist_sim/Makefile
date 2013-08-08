# The Makefile for mist_sim

CC = g++

CFLAGS = -w -std=c++11
LFLAGS = -lGL -lGLU -lglut

all: Mist.o
	$(CC) $(CFLAGS) Mist.o main.cpp -o mist_sim $(LFLAGS)
Mist.o:
	$(CC) $(CFLAGS) -c Particle.h Mist.h Mist.cpp $(LFLAGS)
clean:
	\rm *.o *~ mist_sim