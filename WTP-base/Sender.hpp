#ifndef SENDER_HPP
#define SENDER_HPP

#include <string>
#include <vector>
#include <chrono>
#include "Logger.hpp"
#include "Window.hpp"

class Sender {
public:
    // constructor
    Sender(std::string& receiverIP, int receiverPort, int windowSize, std::string& logFile);

    // Start the connection, send data, and end connection
    void startConnection();
    void sendData(const std::string& filename);
    void endConnection();
private:
    const size_t CHUNK_SIZE = 1456;
    int sockfd;
    sockaddr_in receiverAddr;
    unsigned int sequenceNumber;
    std::unique_ptr<Logger> logger;
    std::unique_ptr<Window> window;
    std::chrono::steady_clock::time_point lastAckTime;

    // helper functions
    int createUDPSocket(int port, std::string& ip);
    void sendNewPacket(const Packet& packet);
    void retransmitPacket(const Packet& packet);
    Packet receiveAck();
    void handleTimeout();
    bool isAckValid(const Packet& ackPacket);
};

#endif // SENDER_HPP