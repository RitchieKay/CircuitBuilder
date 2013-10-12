CC=g++
CFLAGS=-c -Wall
LDFLAGS=
DYNLIB_FLAGS=-shared
SOURCES=CIRabstractComponent.cpp CIRcomplexComponent.cpp CIRmemory.cpp CIRpinArray.cpp CIRbuiltInComponent.cpp CIRcomponentFactory.cpp CIRoscillator.cpp CIRtest.cpp

OBJECTS=$(SOURCES:.cpp=.o)
DYNLIB=libcomponent.so

all: $(SOURCES) $(DYNLIB)
	
$(DYNLIB): $(OBJECTS)
	$(CC) $(DYNLIB_FLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o 
