#
# Makefile
#
CC=g++
# if you want to use debugger, add -g to CFLAGS and LDFLAGS
CFLAGS=-Wall -std=c++11 -O2 -O3
INCLUDES=-I../src/
# INCLUDES=-I../include/
HEADERS=global_routing.h
LFLAGS=
# LFLAGS=-L../lib/
#LIBS=-lm -lsystemc
# LIBS=-lm -lgurobi_c++ -lgurobi70 -fopenmp
SOURCES=main.cpp global_routing.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=../bin/hw5

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJECTS) -o $@ $(LFLAGS) $(LIBS)

%.o:  %.cpp  $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@ 

clean:
	rm -rf *.o $(EXECUTABLE)
