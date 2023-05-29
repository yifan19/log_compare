CXX = g++
CXXFLAGS = -std=c++11 # -Wall

TARGET = compare

SOURCES = main.cpp Event.cpp Log.cpp source.cpp

OBJECTS = $(SOURCES:.cpp=.o)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Clean rule
clean:
	rm -f $(OBJECTS) $(TARGET)
