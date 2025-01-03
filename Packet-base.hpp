#ifndef PACKET_BASE_HPP
#define PACKET_BASE_HPP

#include <vector>
#include <cstdlib>

enum PacketType {
    START = 0,
    END = 1,
    DATA = 2,
    ACK = 3
};

struct PacketHeader {
    unsigned int type;     // 0: START; 1: END; 2: DATA; 3: ACK
    unsigned int seqNum;   // Sequence number
    unsigned int length;   // Length of data; 0 for ACK packets
    unsigned int checkSum; // 32-bit CRC checksum
};

class Packet {
public:
    // Constructor for sending packets (prepares header for network transmission)
    Packet(unsigned int type, const std::vector<char>& data = {}, unsigned int seqNum = 0);

    // Constructor for receiving packets (parses header from received buffer)
    Packet(const char* buffer, size_t bufferSize);

    // Accessor methods
    const PacketHeader& getHeader() const { return header; }
    unsigned int getType() const { return header.type; }
    unsigned int getSeqNum() const { return header.seqNum; }
    unsigned int getLength() const { return header.length; }
    unsigned int getCheckSum() const { return header.checkSum; }
    const std::vector<char>& getData() const { return data; }

    // Checksum calculation method
    unsigned int calculateCheckSum() const;

    // Convert header to network byte order for sending
    PacketHeader getNetworkOrderHeader() const;

private:
    PacketHeader header;
    std::vector<char> data;
};

#endif