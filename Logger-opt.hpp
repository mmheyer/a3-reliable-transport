#ifndef LOGGER_OPT_HPP
#define LOGGER_OPT_HPP

#include <string>
#include <fstream>
#include "Packet-opt.hpp"

class LoggerOpt {
private:
    std::ofstream log_file;

public:
    // Constructor
    LoggerOpt(std::string &log_path);

    // Destructor
    ~LoggerOpt();
    
    // Logs type, seqNum, length, and checkSum of packet header
    bool logPacket(const PacketOptHeader& header);
};

#endif  // LOGGER_HPP
