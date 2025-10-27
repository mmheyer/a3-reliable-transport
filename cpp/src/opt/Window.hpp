#ifndef WINDOW_OPT_HPP
#define WINDOW_OPT_HPP

#include <deque>
#include <vector>
#include "Packet.hpp"

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
    std::deque<Packet>& getPackets();

    // Check if there are any unacknowledged packets in the window
    bool hasPackets() const;

    // Check if a new packet can be added to the window
    bool canAddPacket() const;

    // Check for timed-out packets and return their sequence numbers
    std::vector<unsigned int> getTimedOutPacketSeqNums() const;

    // mark the packet with seqNum as ACKed
    void markPacketAsAcked(unsigned int seqNum);

    size_t determineWindowAdvance();

private:
    int windowSize;
    std::deque<Packet> packets;
};

#endif // WINDOW_HPP
