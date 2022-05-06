#include "EN_TCP_FileSender.h"

void EN_TCP_FileSender::SendFileToServer(std::string FileName, void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
{
	if (IsConnected())
		EN::SendFile(ServerConnectionSocket, FileName, ProgressFunction);
	else std::cerr << "Error: the server is not connected" << std::endl;
	std::cout << "File transfer ended" << std::endl;
}

void EN_TCP_FileSender::RecvFileFromServer()
{
	if (IsConnected())
		EN::RecvFile(ServerConnectionSocket, EN::DownloadStatus);
}

void EN_TCP_FileSender::RecvMessageFromServer(std::string& msg)
{
	if (IsConnected())
		EN::Recv(ServerConnectionSocket, msg);
	else
		std::cerr << "Error! Failed receive message from server" << std::endl;
}
