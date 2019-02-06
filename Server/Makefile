CFLAGS = -g -Wall -Wpedantic
LIBS = -lc
INCLUDES = -I.
HEADERS = Handle.h \
          Router.h \
          Connection.h

OBJECTS = Connection.o \
          Handle.o \
          Router.o

default: main

main: $(OBJECTS) $(HEADERS) main.cpp
	g++ $(CFLAGS) main.cpp $(INCLUDES) $(OBJECTS) $(LIBS) -o main

.cpp.o: 
	g++ -c $(CFLAGS) $(INCLUDES) $<

clean:
	rm -rf $(OBJECTS) main
