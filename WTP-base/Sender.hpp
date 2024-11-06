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

    // destructor
    ~Sender();

    // Start the connection, send data, and end connection
    void startConnection();
    void sendData(const std::string& filename);
    void endConnection();
private:
    int sockfd;
    sockaddr_in receiverAddr;
    unsigned int randSeqNum;
    Logger* logger;
    Window* window;
    // std::chrono::steady_clock::time_point lastAckTime;

    // helper functions
    void createUDPSocket(int receiverPort, std::string& receiverIP);
    void sendPacket(const Packet& packet, bool isFirstSend);
    Packet receiveAck();
    bool isAckValid(const Packet& ackPacket);
};

#endif // SENDER_HPP