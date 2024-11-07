#ifndef WINDOW_BASE_HPP
#define WINDOW_BASE_HPP

#include <deque>
#include <vector>
#include "Packet-base.hpp"

class Window {
public:
    // constructor
    Window(int windowSize) : windowSize(windowSize) {};

    // Get seqNum of the first packet in the window waiting for an ACK
    unsigned int getNextSeqNum() const;

    // add packet to the window
    void addPacket(const Packet& packet);

    // remove packet from the window after receiving
    void removeAcknowledgedPackets(size_t count);

    // access packets in the window
    const std::deque<Packet>& getPackets() const;

    // Check if there are any unacknowledged packets in the window
    bool hasUnacknowledgedPackets() const;

    // Check if a new packet can be added to the window
    bool canAddPacket() const;

private:
    int windowSize;
    std::deque<Packet> packets;
};

#endif // WINDOW_HPP
