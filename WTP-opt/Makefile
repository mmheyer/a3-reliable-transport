# Specify the compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wconversion -Wall -Wextra -pedantic -g

# Executables
SENDER_EXECUTABLE = wSender-opt
RECEIVER_EXECUTABLE = wReceiver-opt
TEST_RECEIVER_EXECUTABLE = testReceiver

# Source files
SENDER_SOURCES = Sender.cpp Logger.cpp Packet.cpp Window.cpp
RECEIVER_SOURCES = wReceiver.cpp Logger.cpp
TEST_RECEIVER_SOURCES = testReceiver.cpp

# Object files
SENDER_OBJECTS = $(SENDER_SOURCES:.cpp=.o)
RECEIVER_OBJECTS = $(RECEIVER_SOURCES:.cpp=.o)
TEST_RECEIVER_OBJECTS = $(TEST_RECEIVER_SOURCES:.cpp=.o)

# Default target to build both executables
all: $(SENDER_EXECUTABLE) $(RECEIVER_EXECUTABLE) $(TEST_RECEIVER_EXECUTABLE)

# Rule for building wSender
$(SENDER_EXECUTABLE): $(SENDER_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SENDER_OBJECTS) -o $(SENDER_EXECUTABLE)

# Rule for building wReceiver
$(RECEIVER_EXECUTABLE): $(RECEIVER_OBJECTS)
	$(CXX) $(CXXFLAGS) $(RECEIVER_OBJECTS) -o $(RECEIVER_EXECUTABLE)

# Rule for building testReceiver
$(TEST_RECEIVER_EXECUTABLE): $(TEST_RECEIVER_OBJECTS)
	$(CXX) $(CXXFLAGS) $(TEST_RECEIVER_OBJECTS) -o $(TEST_RECEIVER_EXECUTABLE)

# Rule for creating object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(SENDER_OBJECTS) $(RECEIVER_OBJECTS) $(TEST_RECEIVER_OBJECTS) \
	      $(SENDER_EXECUTABLE) $(RECEIVER_EXECUTABLE) $(TEST_RECEIVER_EXECUTABLE)

# Phony targets
.PHONY: all clean