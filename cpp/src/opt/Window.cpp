#include <iostream>
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
void Window::removeAcknowledgedPackets(size_t count) {
    for (size_t i = 0; i < count; ++i) {
        packets.pop_front();
    }
}

// Returns a const reference to all packets currently in the window
std::deque<Packet>& Window::getPackets() {
    return packets;
}

bool Window::hasPackets() const {
    return !packets.empty();
}

bool Window::canAddPacket() const {
    return packets.size() < static_cast<unsigned long>(windowSize);
}

// // Check for timed-out packets and return their sequence numbers
// std::vector<unsigned int> Window::getTimedOutPacketSeqNums() const {
//     std::vector<unsigned int> timedOutSeqNums;
//     for (const auto& packetInfo : allPacketInfo) {
//         if (!packetInfo.packetIsAcked() && packetInfo.hasTimedOut()) {
//             timedOutSeqNums.push_back(packetInfo.getPacket().getHeader().seqNum);
//         }
//     }
//     return timedOutSeqNums;
// }

void Window::markPacketAsAcked(unsigned int seqNum) {
    for (auto& packet : packets) {
        if (packet.getSeqNum() == seqNum) {
            packet.setAcked();
            std::cout << "Packet with seqNum " << seqNum << " marked as ACKed." << std::endl;
            return;
        }
    }
    std::cerr << "Packet with seqNum " << seqNum << " not found in the window." << std::endl;
}

// // get packet with seqNum
// const Packet& Window::getPacketWithSeqNum(unsigned int seqNum) const {
//     for (const auto& packet : packets) {
//         if (packet.getSeqNum() == seqNum) {
//             return packet;
//         }
//     }
//     return nullptr; // Return nullptr if the packet is not found
// }

size_t Window::determineWindowAdvance() {
    size_t advanceCount = 0;

    while (!packets.empty() && packets.front().packetIsAcked()) {
        packets.pop_front();
        advanceCount++;
    }

    return advanceCount;
}

// void Window::resetTimerForSeqNum(unsigned int seqNum) {
//     for (auto& packetInfo : allPacketInfo) {
//         if (packetInfo.getPacket().getHeader().seqNum == seqNum) {
//             packetInfo.updateSentTime();
//         }
//     }
// }
