#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <random>
#include "crc32.h"

#define PORT 8888
#define WINDOW_SIZE 3
#define FILE_PREFIX "FILE-"

// Packet loss rate (e.g., 0.2 for 20% loss)
const double PACKET_LOSS_RATE = 0.2;

struct PacketHeader {
    unsigned int type;     // 0: START; 1: END; 2: DATA; 3: ACK
    unsigned int seqNum;   // Sequence number
    unsigned int length;   // Length of data; 0 for ACK packets
    unsigned int checkSum; // 32-bit CRC checksum
};

enum PacketType {
    START = 0,
    END = 1,
    DATA = 2,
    ACK = 3
};

// Calculate CheckSum using starter_files
unsigned int calculateChecksum(const std::vector<char>& data) {
    return crc32(data.data(), data.size());
}

// Test receiver class to handle one connection at a time
class TestReceiver {
public:
    TestReceiver();
    void run();

private:
    int sockfd;
    struct sockaddr_in si_me, si_other;
    socklen_t slen = sizeof(si_other);
    int fileCounter = 0;
    int expectedSeqNum = 0;
    std::ofstream outFile;
    std::mt19937 rng; // Random number generator
    std::uniform_real_distribution<> dist;

    std::string generateFileName();
    bool shouldDropPacket();
    void sendAck(int seqNum);
    void processPacket(const PacketHeader& header, const std::vector<char>& data);
    bool verifyChecksum(const PacketHeader& header, const std::vector<char>& data);
};

TestReceiver::TestReceiver() : dist(0.0, 1.0) {
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket creation failed");
        exit(1);
    }

    memset((char*)&si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
        perror("bind failed");
        close(sockfd);
        exit(1);
    }

    std::cout << "Receiver is listening on port " << PORT << std::endl;

    // Initialize random number generator with a random seed
    rng.seed(std::random_device{}());
}

void TestReceiver::run() {
    char buffer[1472];
    while (true) {
        ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&si_other, &slen);
        if (recv_len < static_cast<int>(sizeof(PacketHeader))) {
            std::cerr << "Received packet is too small" << std::endl;
            continue;
        }

        PacketHeader header;
        std::memcpy(&header, buffer, sizeof(PacketHeader));
        header.type = ntohl(header.type);
        header.seqNum = ntohl(header.seqNum);
        header.length = ntohl(header.length);
        header.checkSum = ntohl(header.checkSum);

        std::vector<char> data(buffer + sizeof(PacketHeader), buffer + recv_len);

        // Simulate packet loss
        if (shouldDropPacket()) {
            std::cout << "Simulating packet loss, dropping packet with seqNum = " << header.seqNum << std::endl;
            continue;
        }

        processPacket(header, data);
    }
}

bool TestReceiver::shouldDropPacket() {
    return dist(rng) < PACKET_LOSS_RATE;
}

void TestReceiver::processPacket(const PacketHeader& header, const std::vector<char>& data) {
    if (header.type == START) {
        if (outFile.is_open()) {
            std::cerr << "Ignored START packet while in an existing connection" << std::endl;
            return;
        }
        outFile.open(generateFileName(), std::ios::binary);
        expectedSeqNum = 0;
        std::cout << "Received START packet. Opening file for new connection." << std::endl;
        sendAck(static_cast<int>(header.seqNum));  // Send ACK with received START seqNum
        return;
    }

    if (header.type == END) {
        if (!outFile.is_open()) {
            std::cerr << "Received END packet with no open file" << std::endl;
            return;
        }
        outFile.close();
        expectedSeqNum = 0;
        fileCounter++;
        std::cout << "Received END packet. Closing file and resetting for new connection." << std::endl;
        sendAck(static_cast<int>(header.seqNum));  // Send ACK with received END seqNum
        return;
    }

    if (header.type == DATA) {
        if (!outFile.is_open()) {
            std::cerr << "DATA packet received outside an active connection" << std::endl;
            return;
        }

        if (header.seqNum >= static_cast<const unsigned int>(expectedSeqNum + WINDOW_SIZE)) {
            std::cerr << "Packet outside window, dropping: seqNum = " << header.seqNum << std::endl;
            return;
        }

        if (!verifyChecksum(header, data)) {
            std::cerr << "Checksum mismatch, dropping packet: seqNum = " << header.seqNum << std::endl;
            return;
        }

        if (header.seqNum == static_cast<const unsigned int>(expectedSeqNum)) {
            outFile.write(data.data(), static_cast<long>(data.size()));
            expectedSeqNum++;
            std::cout << "Received in-order DATA packet: seqNum = " << header.seqNum << std::endl;

            sendAck(expectedSeqNum);  // Cumulative ACK with next expected seqNum
        } else {
            std::cerr << "Out-of-order DATA packet: seqNum = " << header.seqNum << std::endl;
            sendAck(expectedSeqNum);  // Send ACK with current expected seqNum
        }
    }
}

bool TestReceiver::verifyChecksum(const PacketHeader& header, const std::vector<char>& data) {
    return header.checkSum == calculateChecksum(data);
}

void TestReceiver::sendAck(int seqNum) {
    PacketHeader ackHeader;
    ackHeader.type = htonl(ACK);
    ackHeader.seqNum = htonl(seqNum);
    ackHeader.length = 0;
    ackHeader.checkSum = 0;

    if (sendto(sockfd, &ackHeader, sizeof(ackHeader), 0, (struct sockaddr*)&si_other, slen) == -1) {
        perror("sendto failed");
    } else {
        std::cout << "Sent ACK for seqNum = " << seqNum << std::endl;
    }
}

std::string TestReceiver::generateFileName() {
    return FILE_PREFIX + std::to_string(fileCounter) + ".out";
}

int main() {
    TestReceiver receiver;
    receiver.run();
    return 0;
}
