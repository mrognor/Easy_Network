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

	bool EN_TCP_FileSender::SendFileToServer(std::string FileName, void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta), uint64_t previouslySendedBytes, uint64_t microsecondsBetweenSendingChunks)
	{
		std::atomic_bool tmp;
		bool sendRes = EN::SendFile(GetSocket(), FileName, tmp, ProgressFunction, previouslySendedBytes, microsecondsBetweenSendingChunks);
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
		std::atomic_bool tmp;
		return EN::RecvFile(GetSocket(), tmp, ProgressFunction);
	}

	bool EN_TCP_FileSender::ContinueRecvFileFromServer(void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		std::atomic_bool tmp;
		return EN::ContinueRecvFile(GetSocket(), tmp, ProgressFunction);
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
