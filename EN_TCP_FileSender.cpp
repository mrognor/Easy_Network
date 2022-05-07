#include "EN_TCP_FileSender.h"

bool EN_TCP_FileSender::SendFileToServer(std::string FileName, void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
{
	if (IsConnected())
		return EN::SendFile(ServerConnectionSocket, FileName, IsStop, ProgressFunction);
	else std::cerr << "Error: the server is not connected" << std::endl;
	std::cout << "File transfer ended" << std::endl;
	return false;
}

bool EN_TCP_FileSender::RecvFileFromServer()
{
	if (IsConnected())
		return EN::RecvFile(ServerConnectionSocket, IsStop, EN::DownloadStatus);
	return false;
}

bool EN_TCP_FileSender::RecvMessageFromServer(std::string& msg)
{
	if (IsConnected())
	{
		bool IsServerConnected = EN::Recv(ServerConnectionSocket, msg);

		if (IsServerConnected == false)
		{
			std::cerr << "Error! Failed receive message from server" << std::endl;
			return false;
		}
	}
	else
	{
		std::cerr << "Error! Failed receive message from server" << std::endl;
		return false;
	}
	return true;
}
