#include "Packet.hpp"
#include "crc32.h"
#include <vector>

// Constructor for Packet
Packet::Packet(unsigned int type, std::vector<char>& data = {}, unsigned int seqNum = 0)
    : data(data)
{
    header.type = type;
    header.seqNum = seqNum;
    header.length = data.size();
    header.checksum = calculateCheckSum(data);
}

// Calculate CheckSum using starter_files
unsigned int Packet::calculateCheckSum(const std::vector<char>& data) {
    return crc32(data.data(), data.size());
}

// Getter function for seqNum
unsigned int Packet::seqNum() {
    return header.seqNum;
}
