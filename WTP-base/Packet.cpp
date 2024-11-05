#include "Packet.hpp"
#include "crc32.h"
#include <vector>

// Constructor for creating packets to send
Packet::Packet(unsigned int type, const std::vector<char>& data, unsigned int seqNum) : data(data) {
    header.type = htonl(type);                 // Convert type to network byte order
    header.seqNum = htonl(seqNum);             // Convert sequence number to network byte order
    header.length = htonl(data.size());        // Convert data length to network byte order
    header.checkSum = htonl(calculateCheckSum());  // Convert checksum to network byte order
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

// Getter function for seqNum
// unsigned int Packet::seqNum() {
//     return header.seqNum;
// }

// Calculate CheckSum using starter_files
unsigned int Packet::calculateCheckSum() const {
    return crc32(data.data(), data.size());
}

// Constructor for Packet
// TODO Need to add htonl, need to make room for header in data vector
// Packet::Packet(unsigned int type, std::vector<char>& data = {}, unsigned int seqNum = 0)
//     : data(data)
// {
//     header.type = type;
//     header.seqNum = seqNum;
//     header.length = data.size();
//     header.checksum = calculateCheckSum(data);
// }
