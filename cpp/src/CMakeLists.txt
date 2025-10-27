# Create your own CMakeLists.txt file here that will build your project, based 
# on the directory structure you choose. You can use the CMakeLists.txt files
# we provided in the src directory of Project 1 and Project 2 as a starting point.

# You will also have to add a CMakelists.txt file in the common/ directory. 

# Running the standard CMake build sequence should result in the following executables:
#   wSender
#   wReceiver
#   wSenderOpt
#   wReceiverOpt

add_executable(wSender
    base/Packet.cpp
    base/Window.cpp
    base/Sender.cpp
)

add_executable(wReceiver
    base/Window.cpp
    base/Receiver.cpp
)

add_executable(wSenderOpt
    opt/Packet.cpp
    opt/Window.cpp
    opt/Sender.cpp
)

add_executable(wReceiverOpt
    opt/Window.cpp
    opt/Receiver.cpp
)
