#pragma once

#if defined WIN32 || defined _WIN64 

#include <WS2tcpip.h>
#include <Winsock2.h>

#ifndef TCP_KEEPIDLE
#define TCP_KEEPIDLE 3
#endif

#ifndef TCP_KEEPCNT
#define TCP_KEEPCNT 16
#endif

#ifndef TCP_KEEPINTVL
#define TCP_KEEPINTVL 17
#endif

typedef SOCKET EN_SOCKET;

#else

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
typedef int EN_SOCKET; 
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif

#define EN_TCP_KEEPALIVE EN::PredefinedSocketOptions({SOL_SOCKET, IPPROTO_TCP, IPPROTO_TCP, IPPROTO_TCP}, {SO_KEEPALIVE, TCP_KEEPIDLE, TCP_KEEPCNT, TCP_KEEPINTVL}, {1, 1, 1, 1})
#define EN_TCP_NODELAY EN::PredefinedSocketOptions({IPPROTO_TCP}, {TCP_NODELAY}, {1})

#include <vector>

namespace EN
{
    struct SocketOption
    {
        int Level;
        int OptionName;
        int OptionValue;
    };

    /**
        \brief This structure is needed to predefine some convenient socket operation modes. For example, keep alive for convenient work with microcontrollers.
    */
    struct PredefinedSocketOptions
    {
        std::vector<int> Levels;
        std::vector<int> OptionNames;
        std::vector<int> OptionValues;

        PredefinedSocketOptions(std::vector<int> levels, std::vector<int> optionNames, std::vector<int> optionValues);
    };
}


