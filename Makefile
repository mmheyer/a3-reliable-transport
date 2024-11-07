# Specify the compiler and flags
CXX = g++
CXXFLAGS = -std=c++14 -Wconversion -Wall -Werror -Wextra -pedantic -g

# Executables
SENDER_EXECUTABLE = wSender-base
RECEIVER_EXECUTABLE = wReceiver-base
SENDER_OPT_EXECUTABLE = wSender-opt
RECEIVER_OPT_EXECUTABLE = wReceiver-opt
TEST_EXECUTABLE = testReceiver-opt

# Source files
SENDER_SOURCES = Sender-base.cpp Logger-base.cpp Packet-base.cpp Window-base.cpp
SENDER_OPT_SOURCES = Sender-opt.cpp Logger-opt.cpp Packet-opt.cpp Window-opt.cpp
RECEIVER_SOURCES = wReceiver-base.cpp Logger-base.cpp
RECEIVER_OPT_SOURCES = wReceiver-opt.cpp Logger-base.cpp
TEST_SOURCES = testReceiver-opt.cpp

# Object files
SENDER_OBJECTS = $(SENDER_SOURCES:.cpp=.o)
SENDER_OPT_OBJECTS = $(SENDER_OPT_SOURCES:.cpp=.o)
RECEIVER_OBJECTS = $(RECEIVER_SOURCES:.cpp=.o)
RECEIVER_OPT_OBJECTS = $(RECEIVER_OPT_SOURCES:.cpp=.o)
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)

# Default target to build all executables
all: $(SENDER_EXECUTABLE) $(RECEIVER_EXECUTABLE) $(SENDER_OPT_EXECUTABLE) $(RECEIVER_OPT_EXECUTABLE) $(TEST_EXECUTABLE)

# Rule for building wSender
$(SENDER_EXECUTABLE): $(SENDER_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SENDER_OBJECTS) -o $(SENDER_EXECUTABLE)

# Rule for building wSender-opt
$(SENDER_OPT_EXECUTABLE): $(SENDER_OPT_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SENDER_OPT_OBJECTS) -o $(SENDER_OPT_EXECUTABLE)

# Rule for building wReceiver
$(RECEIVER_EXECUTABLE): $(RECEIVER_OBJECTS)
	$(CXX) $(CXXFLAGS) $(RECEIVER_OBJECTS) -o $(RECEIVER_EXECUTABLE)

# Rule for building wReceiver-opt
$(RECEIVER_OPT_EXECUTABLE): $(RECEIVER_OPT_OBJECTS)
	$(CXX) $(CXXFLAGS) $(RECEIVER_OPT_OBJECTS) -o $(RECEIVER_OPT_EXECUTABLE)
	
# Rule for building wReceiver-opt
$(TEST_EXECUTABLE): $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJECTS) -o $(TEST_EXECUTABLE)

# Rule for creating object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f	$(SENDER_OBJECTS) $(RECEIVER_OBJECTS) $(SENDER_OPT_OBJECTS) $(RECEIVER_OPT_OBJECTS) $(TEST_OBJECTS) \
			$(SENDER_EXECUTABLE) $(RECEIVER_EXECUTABLE) $(SENDER_OPT_EXECUTABLE) $(RECEIVER_OPT_EXECUTABLE) $(TEST_EXECUTABLE) 

# Phony targets
.PHONY: all clean
