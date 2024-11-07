#ifndef LOGGER_BASE_HPP
#define LOGGER_BASE_HPP

#include <string>
#include <fstream>
#include "Packet-base.hpp"

class Logger {
private:
    std::ofstream log_file;

public:
    // Constructor
    Logger(std::string &log_path);

    // Destructor
    ~Logger();
    
    // Logs type, seqNum, length, and checkSum of packet header
    bool logPacket(const PacketHeader& header);
};

#endif  // LOGGER_HPP
