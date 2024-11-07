#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>  // For formatting floating-point numbers
#include "Packet.hpp"

// these are the new files :)

// Constructor
Logger::Logger(std::string &log_path) {
    log_file.open(log_path, std::ios::out);
    if (!log_file.is_open()) {
        std::cerr << "Error: Could not open log file: " << log_path << std::endl;
    }
    std::cout << "[DEBUG] Log file opened!\n";
}

// Destructor
Logger::~Logger() {
    log_file.close();
    std::cout << "[DEBUG] Log file closed!\n";
}

// Logs type, seqNum, length, and checkSum of packet header
bool Logger::logPacket(const PacketHeader& header) {
    // write to log_file
    log_file << header.type << " "
             << header.seqNum << " "
             << header.length << " "
             << header.checkSum << " "                                       
             << std::endl; 
    return true;
}
