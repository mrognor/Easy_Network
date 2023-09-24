set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakePredefinedTargets")


if(WIN32)
    set(StandartIncludeLibraries ws2_32)
else()
    set(StandartIncludeLibraries pthread)
endif()

# HTTP server binary
project (HTTP_Server)

add_executable(HTTP_Server Examples/http/HTTP_Server.cpp)

target_include_directories(HTTP_Server PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(HTTP_Server EasyNetwork ${StandartIncludeLibraries})

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

# # RAU chat server binary
# project (RAU_Chat_Server)

# add_executable(RAU_Chat_Server Examples/RAU_Chat_Server.cpp)

# target_include_directories(RAU_Chat_Server PRIVATE ${CMAKE_SOURCE_DIR}/)

# target_link_libraries(RAU_Chat_Server EasyNetwork ${StandartIncludeLibraries})

# # RAU chat server binary
# project (RAU_Chat_Client)

# add_executable(RAU_Chat_Client Examples/RAU_Chat_Client.cpp)

# target_include_directories(RAU_Chat_Client PRIVATE ${CMAKE_SOURCE_DIR}/)

# target_link_libraries(RAU_Chat_Client EasyNetwork ${StandartIncludeLibraries})

# TCP FileSender server binary
project (TCP_BlockingFileTransmitter_Server)

add_executable(TCP_BlockingFileTransmitter_Server Examples/TCP_BlockingFileTransmitter_Server.cpp)

target_include_directories(TCP_BlockingFileTransmitter_Server PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(TCP_BlockingFileTransmitter_Server EasyNetwork ${StandartIncludeLibraries})

# TCP FileSender server binary
project (TCP_BlockingFileTransmitter_Client)

add_executable(TCP_BlockingFileTransmitter_Client Examples/TCP_BlockingFileTransmitter_Client.cpp)

target_include_directories(TCP_BlockingFileTransmitter_Client PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(TCP_BlockingFileTransmitter_Client EasyNetwork ${StandartIncludeLibraries})

# FT server binary
project (FT_Chat_Server)

add_executable(FT_Chat_Server Examples/FT_Chat_Server.cpp)

target_include_directories(FT_Chat_Server PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(FT_Chat_Server EasyNetwork ${StandartIncludeLibraries})

# FT client binary
project (FT_Chat_Client)

add_executable(FT_Chat_Client Examples/FT_Chat_Client.cpp)

target_include_directories(FT_Chat_Client PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(FT_Chat_Client EasyNetwork ${StandartIncludeLibraries})

# Parallel for binary
project (ParallelFor)

add_executable(ParallelFor Examples/ParallelFor.cpp)

target_include_directories(ParallelFor PRIVATE ${CMAKE_SOURCE_DIR}/)

target_link_libraries(ParallelFor EasyNetwork ${StandartIncludeLibraries})

# Library binary 
project(EasyNetwork)

set(LibHeaders EN_Functions.h
EN_TCP_Client.h EN_BackgroundTimer.h EN_FileTransmissionStatus.h
EN_TCP_Server.h EN_UDP_Client.h EN_UDP_Server.h EN_ParallelFor.h EN_SocketOptions.h EN_ThreadGate.h EN_Logger.h 
EN_ThreadCrossWalk.h EN_FT_Server.h EN_FT_Client.h EN_ThreadBarrier.h)

set(LibSources EN_Functions.cpp
EN_TCP_Client.cpp EN_BackgroundTimer.cpp EN_FileTransmissionStatus.cpp
EN_TCP_Server.cpp EN_UDP_Client.cpp EN_UDP_Server.cpp EN_SocketOptions.cpp EN_ThreadGate.cpp EN_Logger.cpp 
EN_ThreadCrossWalk.cpp EN_FT_Server.cpp EN_FT_Client.cpp EN_ThreadBarrier.cpp)

add_library(EasyNetwork STATIC ${LibSources} ${LibHeaders})