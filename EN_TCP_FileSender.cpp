#include "EN_TCP_FileSender.h"

EN_TCP_FileSender::EN_TCP_FileSender() : EN_TCP_Client() {}

// A function to be defined by the user. It is used for logic after connection
void EN_TCP_FileSender::AfterConnect()
{
	IsConnected = true;
	std::cout << "File transfer connection" << std::endl;
}

void EN_TCP_FileSender::SendFileToServer(std::string FileName, void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
{
	if (ServerConnectionSocket != INVALID_SOCKET)
		EN::SendFile(ServerConnectionSocket, FileName, ProgressFunction);
	else std::cerr << "Error: the server is not connected" << std::endl;
	std::cout << "File transfer ended" << std::endl;
}
