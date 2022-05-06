#include "EN_Functions.h"

namespace EN
{
	void Send(EN_SOCKET& sock, std::string message, int MessageDelay)
	{
		int msg_size = message.length();

		send(sock, (char*)&msg_size, sizeof(int), NULL);
		send(sock, message.c_str(), message.length(), NULL);

		#ifdef WIN32
		Sleep(MessageDelay);
		#else
		usleep(MessageDelay);
		#endif
	}

	int Recv(EN_SOCKET& sock, std::string& message)
	{
		int msg_size;
		int ConnectionStatus = recv(sock, (char*)&msg_size, sizeof(int), MSG_WAITALL);

		if (ConnectionStatus <= 0)
		{
			message = "";

			#ifdef WIN32
			closesocket(sock);
			#else 
			close(sock);
			#endif

			return ConnectionStatus;
		}

		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';

		ConnectionStatus = recv(sock, msg, msg_size, MSG_WAITALL);

		if (ConnectionStatus <= 0)
		{
			message = "";

			#ifdef WIN32
			closesocket(sock);
			#else 
			close(sock);
			#endif

			return ConnectionStatus;
		}

		message = msg;
		delete[] msg;
		return ConnectionStatus;
	}

	std::vector<std::string> Split(std::string StringToSplit)
	{
		return Split(StringToSplit, " ");
	}

	std::vector<std::string> Split(std::string StringToSplit, std::string SplitterString)
	{
		std::vector<std::string> ReturnVector;
		int i = 0;
		std::string SplittedString = "";
		while (i < StringToSplit.size())
		{
			if (StringToSplit[i] == SplitterString[0])
			{
				bool IsSplitter = true;
				for (int j = 1; j < SplitterString.size(); j++)
				{
					if (StringToSplit[i + j] != SplitterString[j])
					{
						IsSplitter = false;
						break;
					}
				}
				if (IsSplitter)
				{
					ReturnVector.push_back(SplittedString);
					SplittedString = "";
					i += SplitterString.size();
					continue;
				}
			}
			SplittedString += StringToSplit[i];
			i++;
		}
		ReturnVector.push_back(SplittedString);
		return ReturnVector;
	}

	bool SendFile(EN_SOCKET& FileSendSocket, std::string FileName, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		// Open sending file
		std::ifstream SendingFile(FileName, std::ios::binary);

		// Char array to keep file chunks
		char* MessageBuf = new char[SendFileBufLen];
		memset(MessageBuf, NULL, SendFileBufLen);

		// Find file size in kbytes
		SendingFile.seekg(0, std::ios::end);
		uint64_t FileSize = SendingFile.tellg();

		// Send file name and file size
		EN::Send(FileSendSocket, FileName + " " + std::to_string(FileSize));

		// See file start
		SendingFile.seekg(0, std::ios::beg);

		// Sended kbytes
		uint64_t SendMessageSize = 0;

		int SendBytes;

		if (SendingFile.is_open())
		{
			std::time_t t = std::time(0);
			uint64_t LastSendMessageSize = 0;

			// Skip while loop if file size less when send buffer
			if (FileSize < SendFileBufLen)
				goto SendLittleFile;

			while (SendMessageSize < FileSize - SendFileBufLen)
			{
				if (IsStop == true)
				{
					IsStop = false;
					return false;
				}
				// Print sending status
				if (ProgressFunction != nullptr && std::time(0) - 1 == t)
				{
					//ProgressFunction(SendMessageSize, FileSize, SendMessageSize - LastSendMessageSize, (FileSize - SendMessageSize) / (SendMessageSize - LastSendMessageSize));
					LastSendMessageSize = SendMessageSize;
					t = std::time(0);
				}

				// Read binary data
				SendingFile.read(MessageBuf, SendFileBufLen);
				// Send data to server
				SendBytes = send(FileSendSocket, MessageBuf, SendFileBufLen, 0);

				if (SendBytes <= 0)
				{
					std::cerr << "\nFailed to send file: " << FileName << std::endl;
					SendingFile.close();
					return false;
				}

				// Add sending bytes
				SendMessageSize += SendBytes;
				memset(MessageBuf, NULL, SendFileBufLen);
			}

			// End while loop skipping
		SendLittleFile:

			SendingFile.read(MessageBuf, SendFileBufLen);
			SendBytes = send(FileSendSocket, MessageBuf, SendFileBufLen, 0);

			if (SendBytes <= 0)
			{
				std::cerr << "\nFailed to send file: " << FileName << std::endl;
				SendingFile.close();
				return false;
			}

			if (ProgressFunction != nullptr)
				ProgressFunction(FileSize, FileSize, 0, 0);

			SendingFile.close();
		}

		else
		{
			std::cerr << "\nFailed to open file: " << FileName << std::endl;
			return false;
		}

		delete[] MessageBuf;
		return true;
	}

	bool RecvFile(EN_SOCKET& FileSendSocket, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		// Get file name and file size
		std::string FileInfo;
		if (EN::Recv(FileSendSocket, FileInfo) <= 0)
		{
			std::cerr << "\nFailed to received file name" << std::endl;
			return false;
		}
		

		std::string FileName = Split(FileInfo)[0];
		uint64_t FileSize = std::stoll(Split(FileInfo)[1]);

		// Create received file
		std::ofstream ReceivedFile("r" + FileName, std::ios::binary | std::ios::trunc);
		// Already received bytes
		uint64_t ReceivedMessageSize = 0;

		// Receivef file buffer 
		char* MessageBuf = new char[SendFileBufLen];

		if (ReceivedFile.is_open())
		{
			std::time_t t = std::time(0);
			uint64_t LastReceivedMessageSize = 0;

			// Skip while loop if file size less when send buffer
			if (FileSize < SendFileBufLen)
				goto RecvLittleFile;

			while (ReceivedMessageSize < FileSize - SendFileBufLen)
			{
				if (IsStop == true)
				{
					IsStop = false;
					ReceivedFile.close();
					remove(("r" + FileName).c_str());
					return false;
				}

				if (ProgressFunction != nullptr && std::time(0) - 1 == t)
				{
					ProgressFunction(ReceivedMessageSize, FileSize, ReceivedMessageSize - LastReceivedMessageSize, (FileSize - ReceivedMessageSize) / (ReceivedMessageSize - LastReceivedMessageSize));
					LastReceivedMessageSize = ReceivedMessageSize;
					t = std::time(0);
				}

				int ReceiveBytes = recv(FileSendSocket, MessageBuf, SendFileBufLen, MSG_WAITALL);

				if (ReceiveBytes <= 0)
				{
					std::cerr << "\nFailed to received file: " << FileName << std::endl;
					ReceivedFile.close();
					remove(("r" + FileName).c_str());
					return false;
				}

				ReceivedMessageSize += ReceiveBytes;
				ReceivedFile.write(MessageBuf, SendFileBufLen);
				memset(MessageBuf, NULL, SendFileBufLen);
			}

			// End skipping while loop
		RecvLittleFile:
			int ReceiveBytes = recv(FileSendSocket, MessageBuf, SendFileBufLen, MSG_WAITALL);

			if (ReceiveBytes <= 0)
			{
				std::cerr << "\nFailed to received file: " << FileName << std::endl;
				ReceivedFile.close();
				remove(("r" + FileName).c_str());
				return false;
			}

			ReceivedFile.write(MessageBuf, FileSize);


			if (ProgressFunction != nullptr)
				ProgressFunction(FileSize, FileSize, 0, 0);

			ReceivedFile.close();
		}
		else
		{
			std::cerr << "\nFailed to send file: " << FileName << std::endl;
			return false;
		}
		
		delete[] MessageBuf;
		return true;
	}

	void DownloadStatus(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta)
	{
		std::string AllMeasureName, SpeedName, Eta;
		int AllMeasureSize, SpeedSize, EtaSize;

		AllMeasureName = "KB";
		AllMeasureSize = 1024;
		SpeedName = "KB/s";
		SpeedSize = 1024;
		if (all > 1024 * 1024)
		{
			AllMeasureName = "MB";
			AllMeasureSize = 1024 * 1024;
		}
		if (speed > 1024 * 1024)
		{
			SpeedName = "MB/s";
			SpeedSize = 1024 * 1024;
		}

		if (eta < 60)
		{
			Eta = std::to_string(eta) + " s";
		}
		if (eta > 60 && eta < 3600)
		{
			Eta = std::to_string(eta / 60) + " m";
		}
		if (eta > 3600)
		{
			Eta = std::to_string(eta / 3600) + " h " + std::to_string(eta - (3600 * (eta / 3600)) / 60) + " m";
		}

		std::cout << "\rDownloaded: " << current / AllMeasureSize << " " << AllMeasureName << " All: " << all / AllMeasureSize
			<< " " << (100 * current / all) << " % " << AllMeasureName << " Speed : " << speed / SpeedSize << " " << SpeedName
			<< " ETA : " << Eta << "                                 " << std::flush;

		if (current == all)
			std::cout << "\nEnd file transfering" << std::endl;
	}

	bool IsFileExist(std::string filePath)
	{
		bool isExist = false;
		std::ifstream fin(filePath.c_str());

		if (fin.is_open())
			isExist = true;

		fin.close();
		return isExist;
	}
}