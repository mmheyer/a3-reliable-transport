#include "Packet.hpp"
#include "crc32.h"
#include <vector>

// Constructor for creating packets to send
Packet::Packet(unsigned int type, const std::vector<char>& data, unsigned int seqNum) : data(data) {
    header.type = type;
    header.seqNum = seqNum;
    header.length = static_cast<unsigned int>(data.size());
    header.checkSum = calculateCheckSum();
}

#include "Packet.hpp"

// Constructor for creating packets from received buffer
Packet::Packet(const char* buffer, size_t bufferSize) {
    if (bufferSize < sizeof(PacketHeader)) {
        throw std::runtime_error("Buffer size is too small to contain a valid PacketHeader");
    }

    // Parse each field of PacketHeader individually from the buffer
    header.type = ntohl(*reinterpret_cast<const unsigned int*>(buffer));
    header.seqNum = ntohl(*reinterpret_cast<const unsigned int*>(buffer + 4));
    header.length = ntohl(*reinterpret_cast<const unsigned int*>(buffer + 8));
    header.checkSum = ntohl(*reinterpret_cast<const unsigned int*>(buffer + 12));

    // Copy the data part (if any) from the buffer to the data vector
    if (header.length > 0) {  // header.length represents the length of the data
        data.resize(header.length);
        std::memcpy(data.data(), buffer + sizeof(PacketHeader), header.length);
    }
}

// Calculate CheckSum using starter_files
unsigned int Packet::calculateCheckSum() const {
    return crc32(data.data(), data.size());
}

PacketHeader Packet::getNetworkOrderHeader() const {
    PacketHeader networkOrderHeader;
    networkOrderHeader.type = htonl(header.type);
    networkOrderHeader.seqNum = htonl(header.seqNum);
    networkOrderHeader.length = htonl(header.length);
    networkOrderHeader.checkSum = htonl(header.checkSum);
    return networkOrderHeader;
}
