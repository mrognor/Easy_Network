#pragma once

#if defined WIN32 || defined _WIN64 

#include <WS2tcpip.h>
#include <Winsock2.h>
#define TCP_KEEPIDLE 3
#define TCP_KEEPCNT 16
#define TCP_KEEPINTVL 17

typedef SOCKET EN_SOCKET;

#else

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int EN_SOCKET; 
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif

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

        PredefinedSocketOptions(std::vector<int> levels, std::vector<int> optionNames, std::vector<int> optionValues)
        {
            Levels = levels;
            OptionNames = optionNames;
            OptionValues = optionValues;
        }
    };

    #define TCP_KEEPALIVE PredefinedSocketOptions({SOL_SOCKET, IPPROTO_TCP, IPPROTO_TCP, IPPROTO_TCP}, {SO_KEEPALIVE, TCP_KEEPIDLE, TCP_KEEPCNT, TCP_KEEPINTVL}, {1, 1, 1, 1})
}


