#include "Packet-opt.hpp"
#include "crc32.h"
#include <vector>
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>

// Constructor for creating packets to send
PacketOpt::PacketOpt(unsigned int type, const std::vector<char>& data, unsigned int seqNum) 
    : data(data), isAcked(false), sentTime(std::chrono::steady_clock::now()) {
    header.type = type;
    header.seqNum = seqNum;
    header.length = static_cast<unsigned int>(data.size());
    header.checkSum = calculateCheckSum();
}

// Constructor for creating packets from received buffer
PacketOpt::PacketOpt(const char* buffer, size_t bufferSize) 
    : isAcked(false), sentTime(std::chrono::steady_clock::now()) {
    if (bufferSize < sizeof(PacketOptHeader)) {
        throw std::runtime_error("Buffer size is too small to contain a valid PacketOptHeader");
    }

    // Parse each field of PacketOptHeader individually from the buffer
    header.type = ntohl(*reinterpret_cast<const unsigned int*>(buffer));
    header.seqNum = ntohl(*reinterpret_cast<const unsigned int*>(buffer + 4));
    header.length = ntohl(*reinterpret_cast<const unsigned int*>(buffer + 8));
    header.checkSum = ntohl(*reinterpret_cast<const unsigned int*>(buffer + 12));

    // Copy the data part (if any) from the buffer to the data vector
    if (header.length > 0) {  // header.length represents the length of the data
        data.resize(header.length);
        std::memcpy(data.data(), buffer + sizeof(PacketOptHeader), header.length);
    }
}

// Calculate CheckSum using starter_files
unsigned int PacketOpt::calculateCheckSum() const {
    return crc32(data.data(), data.size());
}

PacketOptHeader PacketOpt::getNetworkOrderHeader() const {
    PacketOptHeader networkOrderHeader;
    networkOrderHeader.type = htonl(header.type);
    networkOrderHeader.seqNum = htonl(header.seqNum);
    networkOrderHeader.length = htonl(header.length);
    networkOrderHeader.checkSum = htonl(header.checkSum);
    return networkOrderHeader;
}

// Check if the packet has timed out (e.g., if more than 500 ms have passed)
bool PacketOpt::hasTimedOut() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - sentTime).count();
    return duration > 500; // Timeout threshold of 500 ms
}
