#pragma once

#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include <string>
#include <vector>
#include <winsock2.h>

namespace EN
{
	// Wrapper over the send function. Allows you to send std::string. Second parametr is delay after send
	void Send(SOCKET& sock, std::string message, int MessageDelay = 10);

	// Wrapper over the recv function. Allows you to recv std::string.
	int Recv(SOCKET& sock, std::string& message);

	// Divides the string to std::vector<std::string>. Second string is string divider
	std::vector<std::string> Split(std::string StringToSplit, std::string SplitterString);
}