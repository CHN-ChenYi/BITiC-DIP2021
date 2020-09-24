TARGET = test

SOURCES = bmp.cpp test.cpp
HEADERS =
OBJECTS = $(SOURCES:.cpp=.o)

CXX = g++
FLAGS = -std=c++11 -O3 -Wall
LIB =
INC =

################################################################################
################################################################################
################################################################################

%.o : %.cpp
	$(CXX) -c $(SOURCES) $(INC) $(FLAGS)

$(TARGET) : $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(LIB)

.PHONY: clean
clean:	
	rm -rf $(OBJECTS) $(TARGET)
