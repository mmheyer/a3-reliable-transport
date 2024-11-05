#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

const int PORT = 8888; // Port to listen on, adjust as necessary
const size_t MAX_BUFFER_SIZE = 1472; // Max size for UDP packet

struct PacketHeader {
    unsigned int type;
    unsigned int seqNum;
    unsigned int length;
    unsigned int checkSum;
};

// Packet types
enum PacketType {
    START = 0,
    END = 1,
    DATA = 2,
    ACK = 3
};

void sendAck(int sockfd, sockaddr_in& clientAddr, socklen_t clientAddrLen, unsigned int seqNum) {
    PacketHeader ackHeader;
    ackHeader.type = htonl(ACK);          // Packet type: ACK
    ackHeader.seqNum = htonl(seqNum);     // Sequence number from the received packet
    ackHeader.length = 0;                 // ACK packet has no data
    ackHeader.checkSum = 0;               // No checksum for this simple example

    sendto(sockfd, &ackHeader, sizeof(ackHeader), 0, (struct sockaddr*)&clientAddr, clientAddrLen);
    std::cout << "Sent ACK for seqNum: " << seqNum << std::endl;
}

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        close(sockfd);
        return 1;
    }

    std::cout << "Receiver listening on port " << PORT << "..." << std::endl;

    while (true) {
        char buffer[MAX_BUFFER_SIZE];
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        ssize_t receivedBytes = recvfrom(sockfd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if (receivedBytes < static_cast<ssize_t>(sizeof(PacketHeader))) {
            std::cerr << "Received packet is too small" << std::endl;
            continue;
        }

        // Parse the packet header
        PacketHeader header;
        std::memcpy(&header, buffer, sizeof(PacketHeader));
        header.type = ntohl(header.type);
        header.seqNum = ntohl(header.seqNum);
        header.length = ntohl(header.length);
        header.checkSum = ntohl(header.checkSum);

        // Print received packet details
        std::cout << "Received packet - Type: " << header.type << ", SeqNum: " << header.seqNum
                  << ", Length: " << header.length << ", Checksum: " << header.checkSum << std::endl;

        // Handle different packet types
        if (header.type == START) {
            std::cout << "Received START packet" << std::endl;
            sendAck(sockfd, clientAddr, clientAddrLen, header.seqNum);
        } else if (header.type == DATA) {
            std::cout << "Received DATA packet" << std::endl;
            sendAck(sockfd, clientAddr, clientAddrLen, header.seqNum); // Acknowledge the data packet
        } else if (header.type == END) {
            std::cout << "Received END packet, closing connection" << std::endl;
            sendAck(sockfd, clientAddr, clientAddrLen, header.seqNum);
            break; // Exit the loop, ending the test
        }
    }

    close(sockfd);
    std::cout << "Receiver closed." << std::endl;
    return 0;
}