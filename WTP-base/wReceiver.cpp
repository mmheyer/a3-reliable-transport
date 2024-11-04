#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <map>
#include "crc32.h"

using namespace std; 


struct packetHeader {
        unsigned int type;      // 0: START; 1: END; 2: DATA; 3: ACK 
        unsigned int seqNum;    // Sequence number
        unsigned int length;    // Length of data; 0 for ACK, START and END                                                       END packets 
        unsigned int checkSum;  // 32-bit CRC	
};

class Packet {
	private:
		vector<char> data;
		

	public:
    	packetHeader header;
        Packet(){
            header.checkSum = 0; 
            header.type = 0;
            header.length = 0; 
            header.seqNum = 0;
        };

		Packet(const char*buffer, ssize_t s){
            //initialize packet info and packet header info
            header.type = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 28)));
            header.seqNum = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 28 + 4)));
            header.length = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 28 + 8)));
            header.checkSum = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 28 + 12)));
            
            data.resize( static_cast<unsigned long>(s) - 20 - sizeof(header));
            memcpy(data.data(), buffer + 28,  static_cast<unsigned long>(s) - 20 - sizeof(header));

        };
};
 
class WReceiver { 
    public: 
        WReceiver(int port, int window_size, string output_dir, string log_file);
        void startReceiving();

    private:
        int sockfd;
        struct sockaddr_in addr, client;
        socklen_t caddr_len;

        int port; 
        int window_size; 
        string output_dir;
        string log_file;

        bool validateChecksum(const char* data, int length, unsigned int receivedChecksum);
        void handlePacket(const packetHeader& header, const char* data, int length);
};

WReceiver::WReceiver(int port, int window_size, string output_dir, string log_file):
    port(port), window_size(window_size), output_dir(output_dir), log_file(log_file){
        //socket, bind, listen
        // initialize socket
        caddr_len = sizeof(client);

        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if(sockfd < 0){
            cout << "Socket Creation Error" << endl;
            exit(1);
        }

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET; 
        addr.sin_addr.s_addr = INADDR_ANY; 
        addr.sin_port = htons(static_cast<uint16_t>(port));

        //bind to port 
        // if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        //     cout << "Bind failed" << endl;
        //     exit(1);
        // }
        bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
}

void WReceiver::startReceiving(){
    bool connection = false; 
    int next_seq = 0; 
    int counter = 0; 
    ofstream output_file;
    int startACK = 0; 
    Packet ackPacket;

    map<int, Packet> receivedPackets; //seq num, packet info
    while(true){
        char buffer[1500]; //8 byte udp header, 20 byte tcp header, 1472 data

        ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &caddr_len);
        if (recv_len < 0) {
            cerr << "Recv error" << endl;
            continue;
        }
        if(recv_len > 28){
            Packet p(buffer, recv_len); //received packet

            //check checksum 
           
            //only handles packet if checksum is right, does nothing if it was wrong
            if(p.header.type == 0){ //START
                //start stuff
                if(connection){ //drops packet
                    continue;
                } else{
                    connection = true; 
                    next_seq = 0; 

                    //ACK FOR START
                    
                    ackPacket.header.type = 3; // 3 = ack
                    startACK = p.header.seqNum;
                    ackPacket.header.seqNum = p.header.seqNum; //ack with start packets seqnum

                    ssize_t bytes_sent = sendto(sockfd, &p, sizeof(p), 0, (struct sockaddr*)&addr, sizeof(addr));
                    if (bytes_sent < 0) {
                        cout << "Failed to send packet" << endl;
                    } else {
                        std::cout << "Sent packet: Type=" << p.header.type << ", SeqNum=" << p.header.seqNum << std::endl;
                    }
                    //TODO: log
                    output_file.open(output_dir + "/FILE-" + std::to_string(counter++) + ".out", std::ios::binary);
                    
                }
            } else if(p.header.type == 1){ //END
                //end
                connection = false;

                //ACK
                ackPacket.header.type = 3; // 3 = ack
                ackPacket.header.seqNum = startACK; //ack with start packets seqnum

                ssize_t bytes_sent = sendto(sockfd, &p, sizeof(p), 0, (struct sockaddr*)&addr, sizeof(addr));
                if (bytes_sent < 0) {
                    cout << "Failed to send packet" << endl;
                } else {
                    std::cout << "Sent packet: Type=" << p.header.type << ", SeqNum=" << p.header.seqNum << std::endl;
                }

                output_file.close();
                //TODO: END
            } else if(p.header.type == 2){ //DATA
                 int c = p.header.checkSum; 
                 int calculated_c = crc32(buffer+28, recv_len -28);
                 if(c == calculated_c){
                    //only do stuff if valid crc
                    if(next_seq + window_size > p.header.seqNum){  //only handles things in windowsize
                        //if packet has correct crc and is in window size-> add to map
                        receivedPackets[p.header.seqNum] = p;
    
                        if(p.header.seqNum == next_seq){
                            while(receivedPackets.find(next_seq) != receivedPackets.end()){
                                //iterates through map
                                Packet order = receivedPackets[next_seq];
                                receivedPackets.erase(next_seq);
                                //TODO write to output file
                                next_seq++;
                            }
                        }
                       
                        ackPacket.header.type = 3; // 3 = ack
                        ackPacket.header.seqNum = next_seq;
                        ssize_t bytes_sent = sendto(sockfd, &p, sizeof(p), 0, (struct sockaddr*)&addr, sizeof(addr));
                        if (bytes_sent < 0) {
                            cout << "Failed to send packet" << endl;
                        } else {
                            std::cout << "Sent packet: Type=" << p.header.type << ", SeqNum=" << p.header.seqNum << std::endl;
                        }
                    }

                    //TODO output dir + log
                    next_seq++;
                 }
            }
                
            

        } else{
            cout << "insufficient buffer size" << endl;
            exit(1);
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
    string output_dir = argv[3];
    string log_file = argv[4];

    WReceiver w(port, window_size, output_dir, log_file);
    w.startReceiving();
    return -1; 
} 
