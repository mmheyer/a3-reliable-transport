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
#include "cxxopts.hpp"
#include <spdlog/spdlog.h>
#include "Sender.hpp"
#include "Window.hpp"
#include "Packet.hpp"

#define BUFLEN 1472

const int PORT = 9000; // Port to listen on, adjust as necessary
const size_t CHUNK_SIZE = 1456;

// constructor
Sender::Sender(std::string& hostname, int port, int window_size)
    : sockfd(0), randSeqNum(0) {
    // setup UDP socket
    createUDPSocket(port, hostname);

    // create window
    window = new Window(window_size);
}

// destructor
Sender::~Sender() {
    // delete logger;
    delete window;
}

// Initiates the connection with a START packet and waits for an ACK
void Sender::startConnection() {
    // randSeqNum = 1 + (std::rand() % 4294967295);

    // Set up the random number generator
    std::random_device rd;  // Seed generator
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    
    // Define the range (0 to max value of unsigned int)
    std::uniform_int_distribution<unsigned int> dis(1, std::numeric_limits<unsigned int>::max());

    // Generate the random sequence number
    randSeqNum = dis(gen);

    // send the start packet
    Packet startPacket(START, {}, randSeqNum);  // Sequence number for START
    sendPacket(startPacket, true);

    // attempt to receive the ACK packet
    Packet ackPacket = receiveAck();

    // if the packet type is 4, recv timed out and we need to retransmit
    if (ackPacket.getType() == 4) {
        std::cerr << "Timed out. Retransmitting packet...\n";
        sendPacket(startPacket, false);
    } 
    // if the checksum or seqnum is incorrect, retransmit the packet
    else if (!isAckValid(ackPacket) || ackPacket.getSeqNum() != startPacket.getSeqNum()) {
        sendPacket(startPacket, false);
    } 
    // otherwise, remove start packet from the window
    else {
        window->removeAcknowledgedPackets(1);
        std::cout << "Connection started successfully.\n";
    }
}

// Reads the file and transmits it in chunks using DATA packets
void Sender::sendData(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file for reading");
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    size_t offset = 0;
    unsigned int dataSeqNum = 0;

    while (offset < buffer.size() || window->hasUnacknowledgedPackets()) {
        // Send packets within the window
        while (window->canAddPacket() && offset < buffer.size()) {
            size_t chunkSize = std::min(CHUNK_SIZE, buffer.size() - offset);
            std::vector<char> chunk(buffer.begin() + static_cast<long>(offset), buffer.begin() + static_cast<long>(offset) + static_cast<long>(chunkSize));

            Packet dataPacket(DATA, chunk, dataSeqNum++);
            sendPacket(dataPacket, true);
            offset += chunkSize;
        }

        // Receive ACKs and handle window movement
        Packet ackPacket = receiveAck();

        // if the packet type is 4, recv timed out and we need to retransmit all the packets
        if (ackPacket.getType() == 4) {
            for (const Packet& packet : window->getPackets()) {
                sendPacket(packet, false);
            }
        }
        // if the checksum is valid and we received an ACK for a packet after expected packet in the window
        else if (isAckValid(ackPacket) && ackPacket.getSeqNum() > window->getNextSeqNum()) {
            size_t ackedPackets = ackPacket.getSeqNum() - static_cast<unsigned int>(window->getNextSeqNum());
            window->removeAcknowledgedPackets(ackedPackets);
        } 
        // checksum was invalid or seqnum was for same packet, so drop the packet
    }
}

// Terminates the connection with an END packet
void Sender::endConnection() {
    Packet endPacket(END, {}, randSeqNum);  // Sequence number for END should match START
    sendPacket(endPacket, true);

    Packet ackPacket = receiveAck();
    // if the packet type is 4, recv timed out so retransmit packet
    if (ackPacket.getType() == 4) {
        std::cerr << "Timed out. Retransmitting packet...\n";
        sendPacket(endPacket, false);
    }
    // if the checksum or seqnum is incorrect, retransmit the packet
    if (!isAckValid(ackPacket) || ackPacket.getSeqNum() != endPacket.getSeqNum()) {
        std::cerr << "Invalid ACK for END packet. Retrying...\n";
        sendPacket(endPacket, false);
    } 
    // otherwise, the connection was ended successfully
    else {
        std::cout << "Connection ended successfully.\n";
    }
}

// create a UDP socket and bind the socket to the port
void Sender::createUDPSocket(int port, std::string& hostname) {
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
    receiverAddr.sin_port = htons(static_cast<uint16_t>(port));

    // Convert the receiver IP to binary form and store in receiverAddr
    if (inet_pton(AF_INET, hostname.c_str(), &receiverAddr.sin_addr) <= 0) {
        close(sockfd);
        throw std::runtime_error("Invalid receiver IP address");
    }

    // Set a 500-millisecond receive timeout
    struct timeval timeout;
    timeout.tv_sec = 0;        // Seconds
    timeout.tv_usec = 500000;  // Microseconds (500 ms)
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(sockfd);
        exit(1);
    }
}

void Sender::sendPacket(const Packet& packet, bool isFirstSend) {
    // Prepare the packet data in a contiguous buffer
    PacketHeader networkHeader = packet.getNetworkOrderHeader();
    size_t totalSize = sizeof(PacketHeader) + packet.getLength();
    std::vector<char> buffer(totalSize);

    std::memcpy(buffer.data(), &networkHeader, sizeof(PacketHeader));
    std::memcpy(buffer.data() + sizeof(PacketHeader), packet.getData().data(), packet.getLength());

    // Send the packet over the socket
    sendto(sockfd, buffer.data(), buffer.size(), 0, (struct sockaddr*)&receiverAddr, sizeof(receiverAddr));
    std::cout << "Packet sent, waiting for ACK..." << std::endl;

    // Log the packet transmission
    // logger->logPacket(packet.getHeader());

    // Add the packet to the window for tracking if this is the first send
    if(isFirstSend) window->addPacket(packet);
}

Packet Sender::receiveAck() {
    // Buffer to hold the incoming packet data
    const size_t bufferSize = sizeof(PacketHeader) + CHUNK_SIZE;
    std::vector<char> buffer(bufferSize);

    // Receive the ACK packet into the buffer
    sockaddr_in senderAddr;
    socklen_t senderAddrLen = sizeof(senderAddr);
    ssize_t receivedBytes = recvfrom(sockfd, buffer.data(), bufferSize, 0, (struct sockaddr*)&senderAddr, &senderAddrLen);

    // Error handling if receiving failed
    if (receivedBytes < 0 && (errno == EWOULDBLOCK || errno == EAGAIN)) {
        std::cout << "Timeout reached, retransmitting packet..." << std::endl;
        // return a packet with a type of 4 to indicate timeout
        Packet noPacket(4);
        return noPacket;
    }
    
    // Create the packet using the data in the buffer
    Packet ackPacket(buffer.data(), bufferSize);

    // log the packet
    // logger->logPacket(ackPacket.getHeader());

    return ackPacket;
}

// Validates the received ACK packet by checking the checksum
bool Sender::isAckValid(const Packet& ackPacket) {
    return ackPacket.getCheckSum() == ackPacket.calculateCheckSum();
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("wSender", "Reliable Sender");
    options.add_options()
        ("h, hostname", "The IP address of the host that wReceiver is running on.", cxxopts::value<std::string>())
        ("p, port", "The port number on which wReceiver is listening.", cxxopts::value<int>())
        ("w, window-size", "Maximum number of outstanding packets in the current window.", cxxopts::value<int>())
        ("i, input-file", "Path to the file that has to be transferred. It can be a text file or binary file (e.g., image or video).", cxxopts::value<std::string>())
        ("o, output-log", "The file path to which you should log the messages as described above.", cxxopts::value<std::string>());

    auto result = options.parse(argc, argv);

    // Extract values
    auto hostname = result["hostname"].as<std::string>();
    auto port = result["port"].as<int>();
    auto window_size = result["window-size"].as<int>();
    auto input_file = result["input-file"].as<std::string>();
    auto output_log = result["output-log"].as<std::string>();

    // Validate arguments
    if (port <= 0 || port > 65535) {
        std::cerr << "Error: Invalid port number. It must be between 1 and 65535.\n";
        return 1;
    }
    if (window_size <= 0) {
        std::cerr << "Error: Window size must be a positive integer.\n";
        return 1;
    }

    // Display parsed arguments for confirmation
    spdlog::debug("hostname: {}", hostname);
    spdlog::debug("port: {}", port);
    spdlog::debug("window_size: {}", window_size);
    spdlog::debug("input_file: {}", input_file);
    spdlog::debug("output_log: {}", output_log);

    // Create Sender
    try {
        Sender sender(hostname, port, window_size);
        sender.startConnection();
        sender.sendData(output_log);
        sender.endConnection();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cout << "Error: " << e.what() << "\n";
    }

    return 0;
}
