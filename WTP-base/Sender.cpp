#include <iostream>
#include <string>
#include <cstdlib>
#include "Sender.hpp"

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: ./wSender <receiver-IP> <receiver-port> <window-size> <input-file> <log>\n";
        return 1;
    }

    // Parse arguments
    std::string receiverIP = argv[1];
    int receiverPort = std::atoi(argv[2]);
    int windowSize = std::atoi(argv[3]);
    std::string inputFile = argv[4];
    std::string logFile = argv[5];

    // Validate arguments
    if (receiverPort <= 0 || receiverPort > 65535) {
        std::cerr << "Error: Invalid port number. It must be between 1 and 65535.\n";
        return 1;
    }
    if (windowSize <= 0) {
        std::cerr << "Error: Window size must be a positive integer.\n";
        return 1;
    }

    // Display parsed arguments for confirmation
    std::cout << "Receiver IP: " << receiverIP << "\n"
              << "Receiver Port: " << receiverPort << "\n"
              << "Window Size: " << windowSize << "\n"
              << "Input File: " << inputFile << "\n"
              << "Log File: " << logFile << "\n";

    // TODO: Further implementation here

    // Create Logger

    // Create Sender

    return 0;
}