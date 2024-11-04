#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <chrono>

// Define TimePoint as a convenience alias for steady_clock time points
using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

// Function to get the current time (for time measurement)
TimePoint get_current_time() {
    return std::chrono::steady_clock::now();
}

// Function to calculate the duration between two time points in seconds
double calculate_duration(const TimePoint& start, const TimePoint& end) {
    return std::chrono::duration<double>(end - start).count();  // Duration in seconds
}

#endif // HELPERS_HPP