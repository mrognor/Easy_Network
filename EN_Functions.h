#pragma once

#ifdef WIN32

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
typedef SOCKET EN_SOCKET;

#else

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int EN_SOCKET;

#endif

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>

#define SendFileBufLen 1024

namespace EN
{
	// Wrapper over the send function. Allows you to send std::string. Second parametr is delay after send
	void TCP_Send(EN_SOCKET& sock, std::string message, int MessageDelay = 10);

	// Wrapper over the recv function. Allows you to recv std::string. Return 0 if socket disconnect, return < 0 if any errors.
	bool TCP_Recv(EN_SOCKET& sock, std::string& message);

	// Divides the string to std::vector<std::string> by " "(space).
	std::vector<std::string> Split(std::string StringToSplit);

	// Divides the string to std::vector<std::string>. Second string is string divider
	std::vector<std::string> Split(std::string StringToSplit, std::string SplitterString);

	// Functions gets socket and filename and send file to socket
	bool SendFile(EN_SOCKET& FileSendSocket, std::string FilePath, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr);

	// This function will wait file
	bool RecvFile(EN_SOCKET& FileSendSocket, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr);

	// This function will print information about file downloading
	void DownloadStatus(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta);

	// Return true if file exists, otherwise rerurn false
	bool IsFileExist(std::string filePath);
}