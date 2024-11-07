#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include <sys/types.h>
#include <cstdio> // For perror()
#include <cerrno> // For errno and strerror()
#include "Logger-base.hpp"
#include "crc32.h"

class Packet1 {
    public:
		std::vector<char> data;
	
    	PacketHeader header;
        Packet1(){
            header.checkSum = 0; 
            header.type = 0;
            header.length = 0; 
            header.seqNum = 0;
        };

		Packet1(const char*buffer, ssize_t s){
            //initialize packet info and packet header info
            header.type = ntohl(*(reinterpret_cast<const unsigned int*>(buffer)));
            header.seqNum = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 4)));
            header.length = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 8)));
            header.checkSum = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 12)));
            
            data.resize( static_cast<unsigned long>(s) - sizeof(header));
            memcpy(data.data(), buffer + sizeof(header),  static_cast<unsigned long>(s) - sizeof(header));

        };

        void toBuffer(char* buffer) const {
            memcpy(buffer, &header.type, sizeof(header.type));
            memcpy(buffer + 4, &header.seqNum, sizeof(header.seqNum));
            memcpy(buffer + 8, &header.length, sizeof(header.length));
            memcpy(buffer + 12, &header.checkSum, sizeof(header.checkSum));
           
        }
        void print() const {
            std::cout << "Packet Info:\n";
            std::cout << "  Type: " << ntohl(header.type) << "\n";
            std::cout << "  Sequence Number: " << ntohl(header.seqNum) << "\n";
            std::cout << "  Length: " << ntohl(header.length) << "\n";
            std::cout << "  Checksum: " << ntohl(header.checkSum) << "\n";
            
        }
    private:
        struct PacketHeader {
        unsigned int type;     // 0: START; 1: END; 2: DATA; 3: ACK
        unsigned int seqNum;   // Sequence number
        unsigned int length;   // Length of data; 0 for ACK packets
        unsigned int checkSum; // 32-bit CRC checksum
    };
};
 
class WReceiver { 
    public: 
        WReceiver(int port, int window_size, std::string output_dir, std::string log_file);
        void startReceiving();

    private:
        int sockfd;
        struct sockaddr_in addr, client;
        socklen_t caddr_len;

        int port; 
        int window_size; 
        std::string output_dir;
        std::string log_file;

        bool validateChecksum(const char* data, int length, unsigned int receivedChecksum);
        void handlePacket(const PacketHeader& header, const char* data, int length);
};

WReceiver::WReceiver(int port, int window_size, std::string output_dir, std::string log_file):
    port(port), window_size(window_size), output_dir(output_dir), log_file(log_file){
        //socket, bind, listen
        // initialize socket
        caddr_len = sizeof(client);

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(sockfd < 0){
            std::cout << "Socket Creation Error" << std::endl;
            exit(1);
        }
        std::cout << "Socket created" << std::endl;

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET; 
        addr.sin_addr.s_addr = INADDR_ANY; 
        addr.sin_port = htons(static_cast<uint16_t>(port));

        //bind to port 
        if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            std::cout << "Bind failed" << std::endl;
            exit(1);
        }
        std::cout << "Bound to port" << std::endl;
}

void writeToFile(const std::vector<char>& data, int index){
    std::string filename = "FILE-" + std::to_string(index) + ".out";
    std::ofstream outfile(filename, std::ios::binary | std::ios::app);
    if(!outfile){
        std::cout << "Failed to open file\n";
        return;
    }
    outfile.write(data.data(), static_cast<long>(data.size()));
    outfile.close();
    std::cout << "[DEBUG] data written to " << filename << std::endl;
}

void WReceiver::startReceiving(){
    bool connection = false; 
    int next_seq = 0; 
    int counter = -1; 
    std::ofstream output_file;
    int startACK = 0; 
    Packet1 ackPacket;
    Logger logOutput(log_file);

    std::cout << "[Debug] Start receiving" << std::endl;
    std::map<int, Packet1> receivedPackets; //seq num, packet info
    while(true){
        char buffer[1500]; //8 byte udp header, 20 byte tcp header, 1472 data
        
        ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &caddr_len);
        if (recv_len < 0) {
            std::cout << "ERROR: Recv error" << std::endl;
            continue;
        }

        Packet1 p(buffer, recv_len); //received packet
        logOutput.logPacket(p.header);
       // next_seq = p.header.seqNum; //next seq is packets seq num
        p.print(); 
        std::cout << "[Debug] initialized packet" << std::endl;

        //check checksum 
        char ACK[1500];

        //only handles packet if checksum is right, does nothing if it was wrong
        if(p.header.type == 0){ //START
            std::cout << "[Start Debug] START received" << std::endl;
            //start stuff
            if(connection){ //drops packet
                std::cout << "Packet dropped due to existing connection/transfer" << std::endl;
                continue;
            } else{
                connection = true; 
                

                //ACK FOR START
                
                ackPacket.header.type = htonl(3); // 3 = ack
                startACK = static_cast<int>(p.header.seqNum);
                logOutput.logPacket(ackPacket.header);
                ackPacket.header.seqNum = htonl(p.header.seqNum); //ack with start packets seqnum

                ackPacket.toBuffer(ACK);
              //  ackPacket.print(); 
                // Send the packet
                ssize_t bytes_sent = sendto(sockfd, ACK, sizeof(ACK), 0, reinterpret_cast<sockaddr*>(&client), caddr_len);
                if (bytes_sent < 0) {
                    std::cout << "ERROR: Failed to send the START ACK\n";
                } else {
                    std::cout << "START ACK sent successfully\n";
                }
                counter++;
                output_file.open(output_dir + "/FILE-" + std::to_string(counter) + ".out", std::ios::binary);
                
                
            }
        } else if(p.header.type == 1){ //END
            std::cout << "[End Debug] END received" << std::endl;
            //end
            connection = false;
            //ACK
            ackPacket.header.type = htonl(3); // 3 = ack
            ackPacket.header.seqNum = htonl(startACK); //ack with start packets seqnum
            ackPacket.toBuffer(ACK);
            //ackPacket.print(); 

            logOutput.logPacket(ackPacket.header);
            ssize_t bytes_sent =  sendto(sockfd, ACK, sizeof(ACK), 0, reinterpret_cast<sockaddr*>(&client), caddr_len);
            if (bytes_sent < 0) {
                std::cout << "ERROR: Failed to send packet" << std::endl;
            } else {
                std::cout << "END ACK sent successfully\n";
            }

            output_file.close();
            //TODO: END
        } else if(p.header.type == 2){ //DATA
            std::cout << "[Data Debug] DATA Received" << std::endl;

             for (ssize_t i = 0; i < recv_len; ++i) {
                if (std::isprint(buffer[i])) {
                    std::cout << (char)buffer[i];  // Print printable characters
                } else {
                    std::cout << ".";  // Print "." for non-printable characters
                }
            }
            std::cout << std::endl;

            int c = static_cast<int>(p.header.checkSum); 
            int calculated_c = static_cast<int>(crc32(p.data.data(), p.data.size()));
            std::cout << c << "*" << calculated_c << std::endl; 
            if(c == calculated_c){
            //only do stuff if valid crc
            next_seq = static_cast<int>(p.header.seqNum);
            if(next_seq + window_size > int(p.header.seqNum) && next_seq <= int(p.header.seqNum)){  //only handles things in windowsize
                
                //if packet has correct crc and is in window size-> add to map
                receivedPackets[static_cast<int>(p.header.seqNum)] = p;
                std::cout << "NEXT SEQ " << next_seq << std::endl;
                std::cout << "HEADEr SWQ " << ntohl(p.header.seqNum) << std::endl;
                //add to map if hasnt been received before
                if (receivedPackets.find(next_seq) == receivedPackets.end()) {
                    receivedPackets[next_seq] = p;
                    writeToFile(p.data, counter); // Writes the valid data to file
                }
                
                ackPacket.header.type = htonl(3); // 3 = ack
                ackPacket.header.seqNum = htonl(next_seq);
                ackPacket.toBuffer(ACK);
                ackPacket.print(); 
                
                logOutput.logPacket(ackPacket.header);
                ssize_t bytes_sent =  sendto(sockfd, ACK, sizeof(ACK), 0, reinterpret_cast<sockaddr*>(&client), caddr_len);
                if (bytes_sent < 0) {
                    std::cout << "Failed to send packet" << std::endl;
                } else {
                    std::cout << "DATA ACK sent successfully\n";
                }
                
            } else{
                std::cout << "packet drop" << std::endl;
            }

            
            }
        }
                
    }
}

int main(int argc, char* argv[]){
    //./wReceiver <port-num> <window-size> <output-dir> <log>

    if (argc != 5) {
        printf("Usage: ./wReceiver <port-num> <window-size> <output-dir> <log>\n");
        return -1;
    }
    int port = atoi(argv[1]);
    int window_size = atoi(argv[2]);
    std::string output_dir = argv[3];
    std::string log_file = argv[4];

    WReceiver w(port, window_size, output_dir, log_file);
    w.startReceiving();
    return -1; 
} 
