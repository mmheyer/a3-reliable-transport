#include <iostream>
#include "Window-opt.hpp"

// get seqNum of packet next to be ACKed
unsigned int WindowOpt::getNextSeqNum() const {
    return allPacketInfo.front().getPacket().getSeqNum();
}

// add packet to the window
void WindowOpt::addPacket(const PacketOpt& packet) {
    allPacketInfo.emplace_back(&packet);
}

// remove packet from the window after receiving
void WindowOpt::removeAcknowledgedPackets(size_t count) {
    for (size_t i = 0; i < count; ++i) {
        allPacketInfo.pop_front();
    }
}

// Returns a const reference to all packets currently in the window
const std::deque<PacketInfo>& WindowOpt::getAllPacketInfo() const {
    return allPacketInfo;
}

bool WindowOpt::hasPackets() const {
    return !allPacketInfo.empty();
}

bool WindowOpt::canAddPacket() const {
    return allPacketInfo.size() < static_cast<unsigned long>(windowSize);
}

// Check for timed-out packets and return their sequence numbers
std::vector<unsigned int> WindowOpt::getTimedOutPacketSeqNums() const {
    std::vector<unsigned int> timedOutSeqNums;
    for (const auto& packetInfo : allPacketInfo) {
        if (!packetInfo.packetIsAcked() && packetInfo.hasTimedOut()) {
            timedOutSeqNums.push_back(packetInfo.getPacket().getHeader().seqNum);
        }
    }
    return timedOutSeqNums;
}

void WindowOpt::markPacketAsAcked(unsigned int seqNum) {
    for (auto& packetInfo : allPacketInfo) {
        if (packetInfo.getPacket().getHeader().seqNum == seqNum) {
            packetInfo.setAcked();
            std::cout << "Packet with seqNum " << seqNum << " marked as ACKed." << std::endl;
            return;
        }
    }
    std::cerr << "Packet with seqNum " << seqNum << " not found in the window." << std::endl;
}

// get packet with seqNum
const PacketOpt* WindowOpt::getPacketWithSeqNum(unsigned int seqNum) const {
    for (const auto& packetInfo : allPacketInfo) {
        if (packetInfo.getPacket().getHeader().seqNum == seqNum) {
            return &packetInfo.getPacket();
        }
    }
    return nullptr; // Return nullptr if the packet is not found
}

size_t WindowOpt::determineWindowAdvance() {
    size_t advanceCount = 0;

    while (!allPacketInfo.empty() && allPacketInfo.front().packetIsAcked()) {
        allPacketInfo.pop_front();
        advanceCount++;
    }

    return advanceCount;
}

void WindowOpt::resetTimerForSeqNum(unsigned int seqNum) {
    for (auto& packetInfo : allPacketInfo) {
        if (packetInfo.getPacket().getHeader().seqNum == seqNum) {
            packetInfo.updateSentTime();
        }
    }
}

