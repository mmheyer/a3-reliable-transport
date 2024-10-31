#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <deque>
#include <vector>
#include "Packet.hpp"

class Window {
public:
    // constructor
    Window(int windowSize) : windowSize(windowSize) {};

    // get seqNum of packet next to be ACKed
    int getNextSeqNum();

    // add packet to the window
    void addPacket(const Packet& packet);

    // remove packet from the window after receiving
    void removeAcknowledgedPackets(size_t count)

    // retransmit all the packets in the window (on timeout)
    void retransmitAll();

private:
    int windowSize;
    std::deque<Packet> packets;
    void retransmit(const Packet& packet);
}

#endif // WINDOW_HPP
