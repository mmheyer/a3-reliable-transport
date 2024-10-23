#include "Logger.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>  // For formatting floating-point numbers

// these are the new files :)

// Constructor
Logger::Logger(std::string &log_path) {
    log_file.open(log_path, std::ios::out);
    if (!log_file.is_open()) {
        std::cerr << "Error: Could not open log file: " << log_path << std::endl;
    }
    std::cout << "[DEBUG] Log file opened!\n";
}
// Destructor
Logger::~Logger() {
    log_file.close();
    std::cout << "[DEBUG] Log file closed!\n";
}
// Initializes the log file (overwrites if it already exists)
// bool Logger::init_log(const std::string& log_path) {
//     // Open file in write mode (to clear), then close
//     std::ofstream log_file;
//     log_file.open(log_path, std::ios::out);  // Overwrite mode
//     if (!log_file.is_open()) {
//         std::cerr << "Error: Could not open log file: " << log_path << std::endl;
//         return false;
//     }
//     std::cout << "[DEBUG] Log file opened!\n";
//     log_file.close();
//     std::cout << "[DEBUG] Log file closed!\n";
//     return true;
// }
// Logs a generic message (for startup events, errors, etc.)
bool Logger::log_message(const std::string& message) {
    // Open in append mode
    // std::ofstream log_file;
    // log_file.open(log_path, std::ios::app);
    // if (!log_file.is_open()) {
    //     std::cerr << "Error: Could not open log file: " << log_path << std::endl;
    //     return false;
    // }
    // Write message to log file
    log_file << message << std::endl;
    //log_file.flush();  // Ensure the message is written to the file immediately
    // Close the log file
    // log_file.close();
    return true;
}
// Logs chunk download activity in the format: 
// <browser-ip> <chunkname> <server-ip> <duration> <tput> <avg-tput> <bitrate>
bool Logger::log_chunk_transfer(const std::string& browser_ip, const std::string& chunkname, const std::string& server_ip,
                        double duration, double tput, double avg_tput, int bitrate) {
    // open log file in append mode
    // std::ofstream log_file;
    // log_file.open(log_path, std::ios::app);
    // if (!log_file.is_open()) {
    //     std::cerr << "Error: Could not open log file: " << log_path << std::endl;
    //     return false;
    // }
    // write to log_file
    log_file << browser_ip << " "
             << chunkname << " "
             << server_ip << " "
             << std::fixed << std::setprecision(3) << duration << " "  // Duration in seconds, 3 decimal places
             << std::fixed << std::setprecision(2) << tput << " "      // Throughput in Kbps, 2 decimal places
             << avg_tput << " "                                        // Average throughput (EWMA) in Kbps
             << bitrate << std::endl;                                  // Requested bitrate in Kbps
    // log_file.flush();  // Ensure immediate writing to the log file
    // close the log file
    // log_file.close();
    return true;
}

// Logs chunk download activity in the format: 
// <browser-ip> <chunkname> <server-ip> <duration> <tput> <avg-tput> <bitrate>
void Logger::log_dns_query(const std::string& client_ip, const std::string& query_name, const std::string& response_ip) {
    // write to log_file
    log_file << client_ip << " " << query_name << " " << response_ip << std::endl;
}
