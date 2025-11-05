#include <chrono>
#include <arpa/inet.h>
#include "Window.hpp"

class Sender {
public:
    // constructor
    Sender(std::string& receiverIP, int receiverPort, int windowSize);

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
    Window* window;

    // helper functions
    void createUDPSocket(int receiverPort, std::string& receiverIP);
    void sendPacket(const Packet& packet, bool isFirstSend);
    Packet receiveAck();
    bool isAckValid(const Packet& ackPacket);
};
