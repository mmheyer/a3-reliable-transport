#ifndef SENDER_HPP
#define SENDER_HPP

#include <string>
#include <vector>
#include <chrono>
#include "Logger.hpp"

class Sender {
public:
    // constructor
    Sender(std::string receiverIP, int receiverPort, int windowSize, std::string inputFile, std::string& logFile);

    // start the sender
    // void run();

    // Start the connection, send data, and end connection
    void startConnection();
    void sendData(const std::string& filename);
    void endConnection();
private:
    int sockfd;
    sockaddr_in receiverAddr;
    int windowSize;
    unsigned int sequenceNumber;
    Logger logger;
    Window window;
    std::chrono::steady_clock::time_point lastAckTime;

    // helper functions
    int createUDPSocket();
    void sendPacket(const Packet& packet);
    Packet receiveAck();
    void handleTimeout();
    unsigned int calculateChecksum(const Packet& packet);
    bool isAckValid(const Packet& ackPacket);
};

#endif // SENDER_HPP