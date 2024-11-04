#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <random>
#include "Sender.hpp"
#include "Logger.hpp"
#include "Window.hpp"
#include "Packet.hpp"
#include "helpers.hpp"

#define BUFLEN 1472

// constructor
Sender::Sender(std::string receiverIP, int receiverPort, int windowSize, Logger* logger)
    : receiverIP(receiverIP), receiverPort(receiverPort) {
    // setup UDP socket
    createUDPSocket(receiverPort, receiverIP);
    
    // create logger
    logger = Logger(logFile);

    // create window
    window = Window(windowSize);
}

// void die(const char *s) {
//     perror(s);
//     exit(1);
// }

// Initiates the connection with a START packet and waits for an ACK
void wSender::startConnection() {
    sequenceNumber = 1 + (std::rand() % 4294967295);

    Packet startPacket(0, {}, sequenceNumber);  // Sequence number for START
    sendPacket(startPacket);

    Packet ackPacket = receiveAck();
    if (!isAckValid(ackPacket) || ackPacket.getSeqNum() != startPacket.getSeqNum()) {
        std::cerr << "Invalid ACK for START packet. Retrying...\n";
        handleTimeout();
    } else {
        std::cout << "Connection started successfully.\n";
    }
}

// Reads the file and transmits it in chunks using DATA packets
void wSender::sendData(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading");
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    size_t offset = 0;

    while (offset < buffer.size() || window.hasUnacknowledgedPackets()) {
        // Send packets within the window
        while (window.canAddPacket() && offset < buffer.size()) {
            size_t chunkSize = std::min(CHUNK_SIZE, buffer.size() - offset);
            std::vector<char> chunk(buffer.begin() + offset, buffer.begin() + offset + chunkSize);

            Packet dataPacket(DATA, chunk, sequenceNumber++);
            sendPacket(dataPacket);
            offset += chunkSize;
        }

        // Receive ACKs and handle window movement
        Packet ackPacket = receiveAck();
        if (isAckValid(ackPacket)) {
            size_t ackedPackets = ackPacket.getSeqNum() - window.getNextSeqNum() + 1;
            window.removeAcknowledgedPackets(ackedPackets);
            lastAckTime = std::chrono::steady_clock::now();
        } else {
            handleTimeout();
        }
    }
}

// Terminates the connection with an END packet
void wSender::endConnection() {
    Packet endPacket(END, {}, sequenceNumber);  // Sequence number for END should match START
    sendPacket(endPacket);

    Packet ackPacket = receiveAck();
    if (!isAckValid(ackPacket) || ackPacket.getSeqNum() != endPacket.getSeqNum()) {
        std::cerr << "Invalid ACK for END packet. Retrying...\n";
        handleTimeout();
    } else {
        std::cout << "Connection ended successfully.\n";
    }
}

// create a UDP socket and bind the socket to the port
int Sender::createUDPSocket(int port, std::string ip) {
    // Create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return -1;
    }

    // Define the server address structure
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;            // IPv4
    serverAddr.sin_port = htons(port);          // Port in network byte order
    serverAddr.sin_addr.s_addr = inet_addr(ip); // Convert IP to network byte order

    // Bind socket to IP address and port
    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(sockfd);
        return -1;
    }

    std::cout << "[DEBUG] UDP socket created and bound to IP: " << ip << ", Port: " << port << std::endl;
    return sockfd;
}

// void Sender::recvMsg() {
//     struct sockaddr_in si_me, si_other;
//     int s, recv_len;
//     socklen_t slen = sizeof(si_other);
//     std::vector<char> buf;
    
//     while(1) {
//         // Try to receive some data, this is a blocking call
//         if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1) {
//             die("recvfrom()");
//         }

//         // Print the details of the client/peer and the data received
//         printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
//         printf("Data: %s\n" , buf);

        
//     }
// }

// Sends a packet and logs the transmission
void wSender::sendPacket(const Packet& packet) {
    // Revisit later. Need to use memcpy, and copy header and data into a buffer and send the buffer.
    sendto(sockfd, &packet, sizeof(Packet), 0, (struct sockaddr*)&receiverAddr, sizeof(receiverAddr));
    logger.logPacket(packet);  // Log the packet information
    window.addPacket(packet);  // Add packet to the window
}

// Receives an ACK and returns it as a Packet
Packet wSender::receiveAck() {
    Packet ackPacket;
    socklen_t addrLen = sizeof(receiverAddr);
    recvfrom(sockfd, &ackPacket, sizeof(Packet), 0, (struct sockaddr*)&receiverAddr, &addrLen);
    return ackPacket;
}

// Handles retransmissions if a timeout occurs
void wSender::handleTimeout() {
    auto currentTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastAckTime);
    if (duration.count() >= 500) {
        std::cerr << "Timeout occurred. Retransmitting all packets in the window.\n";
        window.retransmitAll();
        lastAckTime = std::chrono::steady_clock::now();  // Reset timer
    }
}


// Calculates the checksum for a packet (placeholder for actual CRC implementation)
unsigned int wSender::calculateChecksum(const Packet& packet) {
    // Use a provided CRC function or custom checksum calculation here
    return 0;  // Placeholder
}

// Validates the received ACK packet by checking the checksum
bool wSender::isAckValid(const Packet& ackPacket) {
    return ackPacket.getChecksum() == calculateChecksum(ackPacket);
}

// void Sender::run() {
//     // read input file
//     readInputFile(inputFile);

//     // set up UDP port
//     createUDPSocket(receiverPort, receiverIP);

//     // send start message
//     Packet startPacket(0);
//     sendPacket(startPacket);

//     // start timer for start message
//     start_time = get_current_time();

//     // wait for start ACK

//     // send packets
    
//     // send end message
// }

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: ./wSender <receiver-IP> <receiver-port> <window-size> <input-file> <log>\n";
        return 1;
    }

    // Parse arguments
    std::string receiverIP = argv[1];
    int receiverPort = std::atoi(argv[2]);
    int windowSize = std::atoi(argv[3]);
    std::string inputFile = argv[4];
    std::string logFile = argv[5];

    // Validate arguments
    if (receiverPort <= 0 || receiverPort > 65535) {
        std::cerr << "Error: Invalid port number. It must be between 1 and 65535.\n";
        return 1;
    }
    if (windowSize <= 0) {
        std::cerr << "Error: Window size must be a positive integer.\n";
        return 1;
    }

    // Display parsed arguments for confirmation
    std::cout << "Receiver IP: " << receiverIP << "\n"
              << "Receiver Port: " << receiverPort << "\n"
              << "Window Size: " << windowSize << "\n"
              << "Input File: " << inputFile << "\n"
              << "Log File: " << logFile << "\n";

    // TODO: Further implementation here

    // Create Sender
    try {
        Sender sender(receiverIP, receiverPort, windowSize, inputFile, logFile);
        sender.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cout << "Error: " << e.what() << "\n";
    }

    return 0;
}