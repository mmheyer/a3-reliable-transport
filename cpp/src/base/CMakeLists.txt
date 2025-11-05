add_executable(wSender
    Packet.cpp
    Window.cpp
    Sender.cpp
)

add_executable(wReceiver
    Window.cpp
    Receiver.cpp
)

target_link_libraries(wSender spdlog::spdlog cxxopts::cxxopts)
target_link_libraries(wReceiver spdlog::spdlog cxxopts::cxxopts)