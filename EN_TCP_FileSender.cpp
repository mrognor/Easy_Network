#include "EN_TCP_FileSender.h"

namespace EN
{
	void EN_TCP_FileSender::OnConnect() {};

	void EN_TCP_FileSender::ServerMessageHandler(std::string message) {};

	void EN_TCP_FileSender::OnDisconnect() {};

	EN_TCP_FileSender::EN_TCP_FileSender() : EN_TCP_Client() 
	{
		IsRunMessageHadlerThread = false;
	};

	bool EN_TCP_FileSender::SendFileToServer(std::string FileName, void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta), int DelayInMilliseconds)
	{
		bool sendRes = EN::SendFile(GetSocket(), FileName, IsStop, ProgressFunction, DelayInMilliseconds);
		if (sendRes)
		{
			LOG(Info, "File transfer ended");
		}
		else
		{
			LOG(Warning, "Error: the server is not connected");
		}
		return sendRes;
	}

	bool EN_TCP_FileSender::RecvFileFromServer(void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		return EN::RecvFile(GetSocket(), IsStop, ProgressFunction);
	}

	bool EN_TCP_FileSender::ContinueRecvFileFromServer(void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		return EN::ContinueRecvFile(GetSocket(), IsStop, ProgressFunction);
	}

	bool EN_TCP_FileSender::RecvMessageFromServer(std::string& msg)
	{
		return EN_TCP_Client::WaitMessage(msg);
	}

	void EN_TCP_FileSender::StopDownloading() 
	{ 
		IsStop = true;
	}

	EN_TCP_FileSender::~EN_TCP_FileSender() {}
}
