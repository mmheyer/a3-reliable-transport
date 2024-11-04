#ifndef PACKET_HPP
#define PACKET_HPP

#include <vector>
#include <cstdlib>

struct PacketHeader {
    unsigned int type;     // 0: START; 1: END; 2: DATA; 3: ACK
    unsigned int seqNum;   // Sequence number
    unsigned int length;   // Length of data; 0 for ACK packets
    unsigned int checkSum; // 32-bit CRC checksum
};

class Packet {
public:
    // Constructor
    Packet(unsigned int type, const std::vector<char>& data = {}, unsigned int seqNum = 0);

    // Accessors
    // unsigned int getSeqNum() const;
    // unsigned int getChecksum() const;

private:
    PacketHeader header;
    std::vector<char> data;

    // Checksum calculation method
    unsigned int calculateCheckSum() const;
};

#endif