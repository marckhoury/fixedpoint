OS = $(shell uname -s)

ifeq ($(OS),Darwin)
    CC = clang++
    LIBS = -framework GLUT -framework OpenGL -framework Cocoa
    OPT = -m32 -std=c++11 -stdlib=libc++
else
    CC = g++
    LIBS = -lGL -lglut -lGLU 
    OPT = -O2 -std=c++0x
endif

OBJECTS = io.o mesh.o mathutil.o intersection.o
TARGET = del

del: $(OBJECTS)
	$(CC) $(OPT) -Wno-deprecated-declarations -o $(TARGET) main.cpp $(OBJECTS) $(LIBS)

io.o: io.h io.cpp
	$(CC) $(OPT) -c io.cpp

mesh.o: mesh.h mesh.cpp
	$(CC) $(OPT) -c mesh.cpp

mathutil.o: mathutil.h mathutil.cpp
	$(CC) $(OPT) -c mathutil.cpp

intersection.o: intersection.h intersection.cpp
	$(CC) $(OPT) -c intersection.cpp

clean:
	rm *.o $(TARGET) *~
