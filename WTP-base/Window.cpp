#include "Window.hpp"

// get seqNum of packet next to be ACKed
int Window::getNextSeqNum() {
    return window.top().getSeqNum();
}

// add packet to the window
void Window::addPacket(const Packet& packet) {
    packets.push_back(packet);
}

// remove packet from the window after receiving
void Window::removeAcknowledgedPackets(size_t count) {
    packets.pop_front();
}

// retransmit all the packets in the window (on timeout)
void Window::retransmitAll() {
    for (auto &packet : packets) retransmit(packet);
}

void Window::retransmit(const Packet& packet) {
    // TODO: define retransmission logic here
}
