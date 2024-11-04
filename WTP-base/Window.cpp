#include "Window.hpp"

// get seqNum of packet next to be ACKed
unsigned int Window::getNextSeqNum() const {
    return packets.front().getSeqNum();
}

// add packet to the window
void Window::addPacket(const Packet& packet) {
    packets.push_back(packet);
}

// remove packet from the window after receiving
void Window::removeAcknowledgedPackets() {
    packets.pop_front();
}

// Returns a const reference to all packets currently in the window
const std::deque<Packet>& Window::getPackets() const {
    return packets;
}

bool Window::hasUnacknowledgedPackets() const {
    return !packets.empty();
}

bool Window::canAddPacket() const {
    return packets.size() < static_cast<unsigned long>(windowSize);
}
