set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")


if(WIN32)
    set(StandartIncludeLibraries ws2_32)
else()
    set(StandartIncludeLibraries pthread)
endif()

# TCP chat server binary
project (TCP_Chat_Server)

add_executable(TCP_Chat_Server Examples/TCP_Chat_Server.cpp)

target_include_directories(TCP_Chat_Server PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(TCP_Chat_Server EasyNetwork ${StandartIncludeLibraries})

# TCP chat server binary
project (TCP_Chat_Client)

add_executable(TCP_Chat_Client Examples/TCP_Chat_Client.cpp)

target_include_directories(TCP_Chat_Client PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(TCP_Chat_Client EasyNetwork ${StandartIncludeLibraries})

# UDP chat server binary
project (UDP_Chat_Server)

add_executable(UDP_Chat_Server Examples/UDP_Chat_Server.cpp)

target_include_directories(UDP_Chat_Server PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(UDP_Chat_Server EasyNetwork ${StandartIncludeLibraries})

# UDP chat server binary
project (UDP_Chat_Client)

add_executable(UDP_Chat_Client Examples/UDP_Chat_Client.cpp)

target_include_directories(UDP_Chat_Client PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(UDP_Chat_Client EasyNetwork ${StandartIncludeLibraries})

# RAU chat server binary
project (RAU_Chat_Server)

add_executable(RAU_Chat_Server Examples/RAU_Chat_Server.cpp)

target_include_directories(RAU_Chat_Server PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(RAU_Chat_Server EasyNetwork ${StandartIncludeLibraries})

# RAU chat server binary
project (RAU_Chat_Client)

add_executable(RAU_Chat_Client Examples/RAU_Chat_Client.cpp)

target_include_directories(RAU_Chat_Client PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(RAU_Chat_Client EasyNetwork ${StandartIncludeLibraries})

# TCP FileSender server binary
project (TCP_FileSender_Server)

add_executable(TCP_FileSender_Server Examples/TCP_FileSender_Server.cpp)

target_include_directories(TCP_FileSender_Server PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(TCP_FileSender_Server EasyNetwork ${StandartIncludeLibraries})

# TCP FileSender server binary
project (TCP_FileSender_Client)

add_executable(TCP_FileSender_Client Examples/TCP_FileSender_Client.cpp)

target_include_directories(TCP_FileSender_Client PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(TCP_FileSender_Client EasyNetwork ${StandartIncludeLibraries})

# Parallel for binary
project (ParallelFor)

add_executable(ParallelFor Examples/ParallelFor.cpp)

target_include_directories(ParallelFor PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(ParallelFor EasyNetwork ${StandartIncludeLibraries})

# Library binary 
project(EasyNetwork)

set(LibHeaders EN_Functions.h EN_RAU_Client.h
EN_RAU_Server.h EN_TCP_Client.h  EN_TCP_FileSender.h
EN_TCP_Server.h EN_UDP_Client.h EN_UDP_Server.h EN_ParallelFor.h EN_SocketOptions.h EN_ThreadGate.h EN_Logger.h EN_ThreadCrossWalk.h)

set(LibSources EN_Functions.cpp EN_RAU_Client.cpp
EN_RAU_Server.cpp EN_TCP_Client.cpp  EN_TCP_FileSender.cpp
EN_TCP_Server.cpp EN_UDP_Client.cpp EN_UDP_Server.cpp EN_SocketOptions.cpp EN_ThreadGate.cpp EN_Logger.cpp EN_ThreadCrossWalk.cpp)

add_library(EasyNetwork STATIC ${LibSources} ${LibHeaders})