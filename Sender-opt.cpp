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
#include <fcntl.h>
#include <unistd.h>
#include "Sender-opt.hpp"
#include "Logger-opt.hpp"
#include "Window-opt.hpp"
#include "Packet-opt.hpp"

#define BUFLEN 1472

const int PORT = 9000; // Port to listen on, adjust as necessary
const size_t CHUNK_SIZE = 1456;

// constructor
SenderOpt::SenderOpt(std::string& receiverIP, int receiverPort, int windowSize, std::string& logFile)
    : sockfd(0), randSeqNum(0) {
    // setup UDP socket
    createUDPSocket(receiverPort, receiverIP);
    
    // create logger
    logger = new LoggerOpt(logFile);

    // create window
    window = new WindowOpt(windowSize);
}

// destructor
SenderOpt::~SenderOpt() {
    delete logger;
    delete window;
}

// Initiates the connection with a START packet and waits for an ACK
void SenderOpt::startConnection() {
    // Set up the random number generator
    std::random_device rd;  // Seed generator
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    
    // Define the range (0 to max value of unsigned int)
    std::uniform_int_distribution<unsigned int> dis(1, std::numeric_limits<unsigned int>::max());

    // Generate the random sequence number
    randSeqNum = dis(gen);

    // Send the start packet
    PacketOpt startPacket(STARTOPT, {}, randSeqNum);  // Sequence number for START
    sendPacket(startPacket, true);

    // Timer for 500 ms
    auto startTime = std::chrono::steady_clock::now();
    bool ackReceived = false;

    while (!ackReceived) {
        // Attempt to receive the ACK packet
        PacketOpt ackPacket = receiveAck();

        // Check if the packet type indicates a timeout (type 4 in this context)
        if (ackPacket.getType() == 4) {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();

            if (elapsedTime >= 500) {
                std::cerr << "Timed out. Retransmitting START packet...\n";
                sendPacket(startPacket, false);
                startTime = std::chrono::steady_clock::now(); // Reset the timer after retransmission
            }
        } 
        // If the checksum or seqNum is incorrect, retransmit the packet
        else if (!isAckValid(ackPacket) || ackPacket.getSeqNum() != startPacket.getSeqNum()) {
            std::cerr << "Invalid ACK received. Retransmitting START packet...\n";
            sendPacket(startPacket, false);
            startTime = std::chrono::steady_clock::now(); // Reset the timer after retransmission
        } 
        // If valid ACK is received, mark the connection as started
        else {
            window->removeAcknowledgedPackets(1);
            std::cout << "Connection started successfully.\n";
            ackReceived = true;
        }
    }
}

// Reads the file and transmits it in chunks using DATA packets
void SenderOpt::sendData(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading");
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    size_t offset = 0;
    unsigned int dataSeqNum = 0;

    while (offset < buffer.size() || window->hasPackets()) {
        // Send packets within the window
        while (window->canAddPacket() && offset < buffer.size()) {
            size_t chunkSize = std::min(CHUNK_SIZE, buffer.size() - offset);
            std::vector<char> chunk(buffer.begin() + static_cast<long>(offset), buffer.begin() + static_cast<long>(offset) + static_cast<long>(chunkSize));

            PacketOpt dataPacket(DATAOPT, chunk, dataSeqNum++);
            sendPacket(dataPacket, true);
            offset += chunkSize;
        }

        // Receive ACKs and handle window movement
        PacketOpt ackPacket = receiveAck();

        // if we received an ACK
        if (ackPacket.getType() == ACKOPT) {
            // if the checksum is valid
            if (isAckValid(ackPacket)) {
                // mark the packet as ACKed
                window->markPacketAsAcked(ackPacket.getSeqNum());

                // advance window for any ACKed packets
                window->determineWindowAdvance();
            }

            // we want to check for more ACKs before checking timeouts, so continue to next iteration
            continue;
        }

        // there was no ACK to be received, 
        // so get sequence numbers of the packets that have not been ACKed and timed out
        std::vector<unsigned int> timedOutSeqNums = window->getTimedOutPacketSeqNums();

        // retransmit all packets in the window that have not been ACKed and timed out
        for (unsigned int seqNum : timedOutSeqNums) {
            // retransmit packet
            std::cout << "Retransmitting packet with seq num " << seqNum << std::endl;
            sendPacket(*(window->getPacketWithSeqNum(seqNum)), false);
        }       

        // // if the packet type is 4, recv timed out and we need to retransmit all the packets
        // // now if the packet type is 4, recv time out and we only need to retransmit the packet that timed out
        // if (ackPacket.getType() == 4) {
        //     for (const Packet& packet : window->getPackets()) {
        //         sendPacket(packet, false);
        //     }
        // }
        // // if the checksum is valid and we received an ACK for a packet after expected packet in the window
        // else if (isAckValid(ackPacket) && ackPacket.getSeqNum() > window->getNextSeqNum()) {
        //     size_t ackedPackets = ackPacket.getSeqNum() - static_cast<unsigned int>(window->getNextSeqNum());
        //     window->removeAcknowledgedPackets(ackedPackets);
        // } 
        // // checksum was invalid or seqnum was for same packet, so drop the packet
    }
}

// Terminates the connection with an END packet
void SenderOpt::endConnection() {
    PacketOpt endPacket(ENDOPT, {}, randSeqNum);  // Sequence number for END should match START
    sendPacket(endPacket, true);

    // Timer for 500 ms
    auto startTime = std::chrono::steady_clock::now();
    bool ackReceived = false;

    while (!ackReceived) {
        // Attempt to receive the ACK packet
        PacketOpt ackPacket = receiveAck();

        // Check if the packet type indicates a timeout (type 4 in this context)
        if (ackPacket.getType() == 4) {
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count();

            if (elapsedTime >= 500) {
                std::cerr << "Timed out. Retransmitting END packet...\n";
                sendPacket(endPacket, false);
                startTime = std::chrono::steady_clock::now(); // Reset the timer after retransmission
            }
        }
        // If the checksum or seqNum is incorrect, retransmit the packet
        else if (!isAckValid(ackPacket) || ackPacket.getSeqNum() != endPacket.getSeqNum()) {
            std::cerr << "Invalid ACK for END packet. Retrying...\n";
            sendPacket(endPacket, false);
            startTime = std::chrono::steady_clock::now(); // Reset the timer after retransmission
        }
        // If valid ACK is received, mark the connection as ended
        else {
            std::cout << "Connection ended successfully.\n";
            ackReceived = true;
        }
    }
}

// create a UDP socket and bind the socket to the port
void SenderOpt::createUDPSocket(int receiverPort, std::string& receiverIP) {
    struct sockaddr_in si_me;

    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        perror("socket creation failed");
        exit(1);
    }

    // Zero out the data structure
    memset((char *)&si_me, 0, sizeof(si_me));

    // Set necessary parameters
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to port (if necessary)
    if (bind(sockfd, (struct sockaddr *)&si_me, sizeof(si_me)) == -1) {
        perror("bind failed");
        close(sockfd);
        exit(1);
    }
    std::cout << "UDP socket created and bound to port " << PORT << std::endl;

    // Zero out the receiver address structure
    memset(&receiverAddr, 0, sizeof(receiverAddr));

    // Set up receiver address
    receiverAddr.sin_family = AF_INET;
    receiverAddr.sin_port = htons(static_cast<uint16_t>(receiverPort));

    // Convert the receiver IP to binary form and store in receiverAddr
    if (inet_pton(AF_INET, receiverIP.c_str(), &receiverAddr.sin_addr) <= 0) {
        close(sockfd);
        throw std::runtime_error("Invalid receiver IP address");
    }

    // Set the socket to non-blocking mode using fcntl
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl failed to get flags");
        close(sockfd);
        exit(1);
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl failed to set non-blocking mode");
        close(sockfd);
        exit(1);
    }
}

void SenderOpt::sendPacket(const PacketOpt& packet, bool isFirstSend) {
    // Prepare the packet data in a contiguous buffer
    PacketOptHeader networkHeader = packet.getNetworkOrderHeader();
    size_t totalSize = sizeof(PacketOptHeader) + packet.getLength();
    std::vector<char> buffer(totalSize);

    std::memcpy(buffer.data(), &networkHeader, sizeof(PacketOptHeader));
    std::memcpy(buffer.data() + sizeof(PacketOptHeader), packet.getData().data(), packet.getLength());

    // Send the packet over the socket
    sendto(sockfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&receiverAddr, sizeof(receiverAddr));
    std::cout << "Packet sent, waiting for ACK..." << std::endl;

    // Log the packet transmission
    logger->logPacket(packet.getHeader());

    // If we are sending the packet for the first time
    if (isFirstSend) {
        // add the packet to the window
        window->addPacket(packet);
    }
    // otherwise, we are retransmitting
    else {
        // reset the timer for that packet
        window->resetTimerForSeqNum(packet.getSeqNum());
    }
}

PacketOpt SenderOpt::receiveAck() {
    // Buffer to hold the incoming packet data
    const size_t bufferSize = sizeof(PacketOptHeader) + CHUNK_SIZE;
    std::vector<char> buffer(bufferSize);

    // Receive the ACK packet into the buffer
    sockaddr_in senderAddr;
    socklen_t senderAddrLen = sizeof(senderAddr);
    ssize_t receivedBytes = recvfrom(sockfd, buffer.data(), bufferSize, 0, (struct sockaddr*)&senderAddr, &senderAddrLen);

    // Error handling if receiving failed
    if (receivedBytes < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
        // No ACKs are being received, so return a packet with a type of 4 to indicate timeout
        PacketOpt noPacket(4);
        return noPacket;
    }
    
    // Create the packet using the data in the buffer
    PacketOpt ackPacket(buffer.data(), bufferSize);

    // log the packet
    logger->logPacket(ackPacket.getHeader());

    return ackPacket;
}

// void Sender::sendNewPacket(const PacketOpt& packet) {
//     // Prepare the packet data in a contiguous buffer
//     PacketHeader networkHeader = packet.getNetworkOrderHeader();
//     size_t totalSize = sizeof(PacketHeader) + packet.getLength();
//     std::vector<char> buffer(totalSize);

//     std::memcpy(buffer.data(), &networkHeader, sizeof(PacketHeader));
//     std::memcpy(buffer.data() + sizeof(PacketHeader), packet.getData().data(), packet.getLength());

//     // Send the packet over the socket
//     sendto(sockfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&receiverAddr, sizeof(receiverAddr));

//     // Log the packet transmission
//     logger->logPacket(packet.getHeader());

//     // Add the packet to the window for tracking since this is the first send
//     window->addPacket(packet);
// }

// void Sender::retransmitPacket(const PacketOpt& packet) {
//     std::cout << "Retransmitting packet with sequence number: " << packet.getSeqNum() << std::endl;

//     // Prepare the packet data in a contiguous buffer (same as in sendNewPacket)
//     PacketHeader networkHeader = packet.getNetworkOrderHeader();
//     size_t totalSize = sizeof(PacketHeader) + packet.getLength();
//     std::vector<char> buffer(totalSize);

//     std::memcpy(buffer.data(), &networkHeader, sizeof(PacketHeader));
//     std::memcpy(buffer.data() + sizeof(PacketHeader), packet.getData().data(), packet.getLength());

//     // Send the packet over the socket
//     sendto(sockfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&receiverAddr, sizeof(receiverAddr));

//     // Log the retransmission, but do not add to the window again
//     logger->logPacket(packet.getHeader());
// }

// // Receives an ACK and returns it as a PacketOpt
// PacketOpt Sender::receiveAck() {
//     // Buffer to hold the incoming packet data
//     const size_t bufferSize = sizeof(PacketHeader) + CHUNK_SIZE;
//     std::vector<char> buffer(bufferSize);

//     // Receive the ACK packet into the buffer
//     sockaddr_in senderAddr;
//     socklen_t senderAddrLen = sizeof(senderAddr);
//     ssize_t receivedBytes = recvfrom(sockfd, buffer.data(), bufferSize, 0, (struct sockaddr*)&senderAddr, &senderAddrLen);

//     // Error handling if receiving failed
//     if (receivedBytes < static_cast<ssize_t>(sizeof(PacketHeader))) {
//         throw std::runtime_error("Failed to receive a complete ACK packet");
//     }
    
//     // Create the packet using the data in the buffer
//     PacketOpt ackPacket(buffer.data(), bufferSize);

//     return ackPacket;
// }

// void Sender::handleTimeout() {
//     auto currentTime = std::chrono::steady_clock::now();
//     auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastAckTime);
//     if (duration.count() >= 500) {
//         std::cout << "Timeout occurred. Retransmitting all packets in the window.\n";
        
//         // Retransmit all packets in the window
//         for (const PacketOpt& packet : window->getPackets()) {
//             retransmitPacket(packet);
//         }

//         // Reset the timer
//         lastAckTime = std::chrono::steady_clock::now();
//     }
// }

// Validates the received ACK packet by checking the checksum
bool SenderOpt::isAckValid(const PacketOpt& ackPacket) {
    return ackPacket.getCheckSum() == ackPacket.calculateCheckSum();
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: ./SenderOpt <receiver-IP> <receiver-port> <window-size> <input-file> <log>\n";
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

    // Create Sender
    try {
        SenderOpt sender(receiverIP, receiverPort, windowSize, logFile);
        sender.startConnection();
        sender.sendData(inputFile);
        sender.endConnection();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cout << "Error: " << e.what() << "\n";
    }

    return 0;
}

// // Calculates the checksum for a packet (placeholder for actual CRC implementation)
// unsigned int Sender::calculateChecksum(const Packet& packet) {
//     // Use a provided CRC function or custom checksum calculation here
//     return crc32(packet->data.data(), packet->data.size());  // Placeholder
// }

// void die(const char *s) {
//     perror(s);
//     exit(1);
// }

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

// void Sender::sendPacket(const Packet& packet) {
//     // Calculate the total size: header + data
//     size_t totalSize = sizeof(PacketHeader) + packet.data.size();

//     // Create a buffer to hold the entire packet
//     std::vector<char> buffer(totalSize);

//     // Copy the header into the buffer
//     std::memcpy(buffer.data(), &packet.header, sizeof(PacketHeader));

//     // Copy the data into the buffer after the header
//     std::memcpy(buffer.data() + sizeof(PacketHeader), packet.data.data(), packet.data.size());

//     // Send the contiguous buffer over the socket
//     sendto(sockfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&receiverAddr, sizeof(receiverAddr));

//     // Log and add to the window as before
//     logger.logPacket(packet);
//     window.addPacket(packet);
// }

// // Retransmit all packets currently in the window
// void Sender::retransmitAll() {
//     std::cout << "Retransmitting all packets in the window...\n";
//     for (const Packet& packet : window.getPackets()) {
//         retransmit(packet);
//     }
// }
