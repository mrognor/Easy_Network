#pragma once

#ifdef WIN32

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
typedef SOCKET EN_SOCKET;

#else

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int EN_SOCKET;

#endif

#include <iostream>
#include <string>
#include <vector>

namespace EN
{
	// Wrapper over the send function. Allows you to send std::string. Second parametr is delay after send
	void Send(EN_SOCKET& sock, std::string message, int MessageDelay = 10);

	// Wrapper over the recv function. Allows you to recv std::string.
	int Recv(EN_SOCKET& sock, std::string& message);

	// Divides the string to std::vector<std::string>. Second string is string divider
	std::vector<std::string> Split(std::string StringToSplit, std::string SplitterString);

	void SendFile(SOCKET& FileSendSocket, std::string FilePath);
}