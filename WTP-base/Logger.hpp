#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>

class Logger {
private:
    std::ofstream log_file;

public:
    // Constructor
    Logger(std::string &log_path);

    // Destructor
    ~Logger();
    
    // Logs type, seqNum, length, and checkSum of packet header
    bool logPacket(packetHeader& header);
};

#endif  // LOGGER_HPP
