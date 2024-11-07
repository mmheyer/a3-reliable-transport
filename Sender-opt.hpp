#ifndef SENDER_OPT_HPP
#define SENDER_OPT_HPP

#include <string>
#include <vector>
#include <chrono>
#include "Logger-opt.hpp"
#include "Window-opt.hpp"

class SenderOpt {
public:
    // constructor
    SenderOpt(std::string& receiverIP, int receiverPort, int windowSize, std::string& logFile);

    // destructor
    ~SenderOpt();

    // Start the connection, send data, and end connection
    void startConnection();
    void sendData(const std::string& filename);
    void endConnection();
private:
    int sockfd;
    sockaddr_in receiverAddr;
    unsigned int randSeqNum;
    LoggerOpt* logger;
    WindowOpt* window;

    // helper functions
    void createUDPSocket(int receiverPort, std::string& receiverIP);
    void sendPacket(const PacketOpt& packet, bool isFirstSend);
    PacketOpt receiveAck();
    bool isAckValid(const PacketOpt& ackPacket);
};

#endif // SENDER_HPP