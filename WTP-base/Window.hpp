#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <deque>
#include <vector>
#include "Packet.hpp"

class Window {
public:
    // constructor
    Window(int windowSize) : windowSize(windowSize), baseSeqNum(0) {};

    // Get seqNum of the first packet in the window waiting for an ACK
    int getNextSeqNum() const;

    // add packet to the window
    void addPacket(const Packet& packet);

    // remove packet from the window after receiving
    void removeAcknowledgedPackets(size_t count);

    // retransmit all the packets in the window (on timeout)
    void retransmitAll();

private:
    int windowSize;
    int baseSeqNum;
    std::deque<Packet> packets;
    
    void retransmit(const Packet& packet); // Helper to retransmit a packet
}

#endif // WINDOW_HPP
