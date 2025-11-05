add_executable(wSenderOpt
    Packet.cpp
    Window.cpp
    Sender.cpp
)

add_executable(wReceiverOpt
    Window.cpp
    Receiver.cpp
)

target_link_libraries(wSenderOpt spdlog::spdlog cxxopts::cxxopts)
target_link_libraries(wReceiverOpt spdlog::spdlog cxxopts::cxxopts)