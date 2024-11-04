#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
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

		Packet(const char*buffer, ssize_t s){
            //initialize packet info and packet header info
            header.type = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 28)));
            header.seqNum = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 28 + 4)));
            header.length = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 28 + 8)));
            header.checkSum = ntohl(*(reinterpret_cast<const unsigned int*>(buffer + 28 + 12)));
            
            data.resize( s - 20 - sizeof(header));
            memcpy(data.data(), buffer + 28,  s - 20 - sizeof(header));

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
        if (bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
            cout << "Bind failed" << endl;
            exit(1);
        }
}

void WReceiver::startReceiving(){
    while(true){
        char buffer[1500]; //8 byte udp header, 20 byte tcp header, 1472 data

        ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client, &caddr_len);
        if (recv_len < 0) {
            cerr << "Recv error" << endl;
            continue;
        }
        if(recv_len > 28){
            Packet p(buffer, recv_len); 

            //check checksum 
            int c = p.header.checkSum; 
            int calculated_c = crc32(buffer+28, recv_len -28);
            if(c == calculated_c){
                //only handles packet if checksum is right, does nothing if it was wrong

                
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