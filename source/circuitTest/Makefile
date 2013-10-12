CC=g++
CFLAGS=-c -Wall
LDFLAGS=-L../components -lcomponent
INCFLAGS=-I../components 
SOURCES=CIRtest.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=CIRtest

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) $(INCFLAGS) $< -o $@

clean:
	rm *.o 
