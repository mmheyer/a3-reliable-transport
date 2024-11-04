# Specify the compiler and flags
CXX = g++
CXXFLAGS = -std=c++14 -Wconversion -Wall -Werror -Wextra -pedantic 

# Executables
SENDER_EXECUTABLE = wSender-base
RECEIVER_EXECUTABLE = wReceiver-base

# Source files
SENDER_SOURCES = Sender.cpp Logger.cpp Packet.cpp Window.cpp
RECEIVER_SOURCES = wReceiver.cpp Logger.cpp

# Object files
SENDER_OBJECTS = $(SENDER_SOURCES:.cpp=.o)
RECEIVER_OBJECTS = $(RECEIVER_SOURCES:.cpp=.o)

# Default target to build both executables
all: $(SENDER_EXECUTABLE) $(RECEIVER_EXECUTABLE)

# Rule for building wSender
$(SENDER_EXECUTABLE): $(SENDER_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SENDER_OBJECTS) -o $(SENDER_EXECUTABLE)

# Rule for building wReceiver
$(RECEIVER_EXECUTABLE): $(RECEIVER_OBJECTS)
	$(CXX) $(CXXFLAGS) $(RECEIVER_OBJECTS) -o $(RECEIVER_EXECUTABLE)

# Rule for creating object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(SENDER_OBJECTS) $(RECEIVER_OBJECTS) $(SENDER_EXECUTABLE) $(RECEIVER_EXECUTABLE)

# Phony targets
.PHONY: all clean