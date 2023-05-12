#include "EN_TCP_FileSender.h"

namespace EN
{
	bool EN_TCP_FileSender::SendFileToServer(std::string FileName, void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta), int DelayInMilliseconds)
	{
		if (IsConnected())
			return EN::SendFile(GetSocket(), FileName, IsStop, ProgressFunction, DelayInMilliseconds);
		else LOG(Warning, "Error: the server is not connected");
        LOG(Message, "File transfer ended");
		return false;
	}

	bool EN_TCP_FileSender::RecvFileFromServer(void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		if (IsConnected())
			return EN::RecvFile(GetSocket(), IsStop, ProgressFunction);
		return false;
	}

	bool EN_TCP_FileSender::ContinueRecvFileFromServer(void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		if (IsConnected())
			return EN::ContinueRecvFile(GetSocket(), IsStop, ProgressFunction);
		return false;
	}

	void EN_TCP_FileSender::RecvMessageFromServer(std::string& msg)
	{
		if (IsConnected())
			bool IsServerConnected = EN::TCP_Recv(GetSocket(), msg);
	}
}
