#ifndef PACKET_OPT_HPP
#define PACKET_OPT_HPP

#include <vector>
#include <cstdlib>
#include <chrono>

// Define TimePoint as a convenience alias for steady_clock time points
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

enum PacketOptType {
    STARTOPT = 0,
    ENDOPT = 1,
    DATAOPT = 2,
    ACKOPT = 3,
    ERROROPT = 4
};

struct PacketOptHeader {
    unsigned int type;     // 0: START; 1: END; 2: DATA; 3: ACK
    unsigned int seqNum;   // Sequence number
    unsigned int length;   // Length of data; 0 for ACK packets
    unsigned int checkSum; // 32-bit CRC checksum
};

class PacketOpt {
public:
    // Constructor for sending packets (prepares header for network transmission)
    PacketOpt(unsigned int type, const std::vector<char>& data = {}, unsigned int seqNum = 0);

    // Constructor for receiving packets (parses header from received buffer)
    PacketOpt(const char* buffer, size_t bufferSize);

    // Accessor methods
    const PacketOptHeader& getHeader() const { return header; }
    unsigned int getType() const { return header.type; }
    unsigned int getSeqNum() const { return header.seqNum; }
    unsigned int getLength() const { return header.length; }
    unsigned int getCheckSum() const { return header.checkSum; }
    const std::vector<char>& getData() const { return data; }

    // Checksum calculation method
    unsigned int calculateCheckSum() const;

    // Convert header to network byte order for sending
    PacketOptHeader getNetworkOrderHeader() const;

private:
    PacketOptHeader header;
    std::vector<char> data;
};

class PacketInfo {
public:
    // Constructor to create a PacketInfo with the current time as sentTime
    PacketInfo(const PacketOpt* packet) 
        : packet(packet), isAcked(false), sentTime(std::chrono::steady_clock::now()) {}

    // Accessor for the packet
    const PacketOpt& getPacket() const { return *packet; }

    // Mark the packet as acknowledged
    void setAcked() { isAcked = true; }

    // Check if the packet is acknowledged
    bool packetIsAcked() const { return isAcked; }

    // Update the sent time (e.g., for retransmission)
    void updateSentTime() { sentTime = std::chrono::steady_clock::now(); }

    // Check if the packet has timed out (e.g., if more than 500 ms have passed)
    bool hasTimedOut() const;

    // getter for sent time
    TimePoint getSentTime() { return sentTime; }
private:
    const PacketOpt* packet;
    bool isAcked;
    TimePoint sentTime;
};

#endif