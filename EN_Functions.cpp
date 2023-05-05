#include "EN_Functions.h"

namespace EN
{
	std::string GetIpByURL(std::string url)
	{
		// First is gethostbyname. Return hostent struct
		// Second is h_addr_list from hostent struct
		// Fird is type conversion from h_addr_list[0] to in_addr*
		// Fourth is get data from in_addr pointer 
		// Fifth is convert in_addr to char* using inet_ntoa
		auto addr = gethostbyname(url.c_str());
		if (addr != nullptr)
			return std::string(inet_ntoa(*(in_addr*)addr->h_addr_list[0]));
		else
			return "";
	}

	void TCP_Send(EN_SOCKET sock, std::string message, int MessageDelay)
	{
		size_t messageLength = message.length();
		unsigned char* msgBuf;
		unsigned char messageByteLength;

		// If message length more than 128, then set first bit to 1, that means that message
		// length stored in 2 bytes
		// If message length less or equal 128, than set first bit to 0, and send only 
		// one byte with message length
		if (messageLength >= 128)
		{
			msgBuf = new unsigned char[2  + messageLength];
			msgBuf[0] = messageLength / 128;
			msgBuf[0] |= 0b10000000;
			msgBuf[1] = messageLength % 128;
			messageByteLength = 2;
		}
		else
		{
			msgBuf = new unsigned char[1  + messageLength];
			msgBuf[0] = messageLength;
			messageByteLength = 1;
		}		

		for (size_t i = 0; i < messageLength; ++i)
			msgBuf[i + messageByteLength] = message[i];

		send(sock, (char*)msgBuf, messageLength + messageByteLength, 0);

		Delay(MessageDelay);
	}

	bool TCP_Recv(EN_SOCKET sock, std::string& message)
	{
		int msg_size;
		unsigned char firstMessageLengthByte;
		int ConnectionStatus = recv(sock, (char*)&firstMessageLengthByte, 1, MSG_WAITALL);

		if (ConnectionStatus <= 0)
		{
			message = "";

			CloseSocket(sock);
			return false;
		}
		
		// If first bit equls 1, then message length store in 2 bytes and we need to read next byte
		if (firstMessageLengthByte & 0b10000000)
		{
			unsigned char secondMessageLengthByte;
			ConnectionStatus = recv(sock, (char*)&secondMessageLengthByte, 1, MSG_WAITALL);

			if (ConnectionStatus <= 0)
			{
				message = "";

				CloseSocket(sock);
				return false;
			}	

			msg_size = (firstMessageLengthByte & 0b01111111) * 128;
			msg_size += secondMessageLengthByte & 0b01111111;
		}
		else
			msg_size = firstMessageLengthByte & 0b01111111;
		
		if (msg_size <= 0)
		{
			message = "";
			return true;
		}

		char* msg = new char[msg_size];

		ConnectionStatus = recv(sock, msg, msg_size, MSG_WAITALL);

		if (ConnectionStatus <= 0)
		{
			message = "";

			CloseSocket(sock);
			delete[] msg;
			return false;
		}

	    message.clear();
	    for (int i = 0; i < msg_size; ++i)
		    message += msg[i];

		delete[] msg;
		return true;
	}

	void CloseSocket(EN_SOCKET sock)
	{
		#if defined WIN32 || defined _WIN64
		closesocket(sock);
		#else 
		close(sock);
		#endif
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

	bool SendFile(EN_SOCKET FileSendSocket, std::string FileName, bool& IsStop,
		void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta), 
		uint64_t PreviouslySendedSize, int ChunksNumberBetweenDelay)
	{	
		// Open sending file
		std::ifstream SendingFile(FileName, std::ios::binary);

		// Char array to keep file chunks
		char* MessageBuf = new char[SendFileBufLen];
		memset(MessageBuf, 0, SendFileBufLen);

		// Find file size in bytes
		SendingFile.seekg(0, std::ios::end);
		uint64_t FileSize = SendingFile.tellg();

		// Send file name and file size
		EN::TCP_Send(FileSendSocket, FileName + " " + std::to_string(FileSize - PreviouslySendedSize));

		// See file start
		SendingFile.seekg(0, std::ios::beg);

		// Sended bytes
		uint64_t SendMessageSize = 0;

		// Current iteration sended bytes
		int SendBytes;
		
		// Amount of sended chunks previously
		uint64_t BeforeSendedChunks = PreviouslySendedSize / SendFileBufLen;
		uint64_t ChunksSended = 0;

		if (SendingFile.is_open())
		{
			std::time_t t = std::time(0);
			uint64_t LastSendMessageSize = 0;
			int SpeedStep = 0;

			// Skip while loop if file size less when send buffer
			if (FileSize < SendFileBufLen)
				goto SendLittleFile;

			while (SendMessageSize < FileSize - SendFileBufLen)
			{
				if (ChunksSended < BeforeSendedChunks)
				{
					SendingFile.seekg(SendFileBufLen, std::ios::cur);
					SendMessageSize += SendFileBufLen;
					ChunksSended++;
					continue;
				}

				if (IsStop == true)
				{
					IsStop = false;
					delete[] MessageBuf;
					return false;
				}
				// Print sending status
				if (ProgressFunction != nullptr && std::time(0) > t)
				{
					ProgressFunction(SendMessageSize, FileSize, SendMessageSize - LastSendMessageSize, (FileSize - SendMessageSize) / (SendMessageSize - LastSendMessageSize));
					LastSendMessageSize = SendMessageSize;
					t = std::time(0);
				}

				// Regulate transfering speed
				if (ChunksNumberBetweenDelay > 0)
				{
					if (SpeedStep > ChunksNumberBetweenDelay)
					{
						EN::Delay(20);
						SpeedStep = 0;
					}
					else
						SpeedStep++;
				}

				// Read binary data
				SendingFile.read(MessageBuf, SendFileBufLen);
				// Send data to server
				SendBytes = send(FileSendSocket, MessageBuf, SendFileBufLen, 0);

				if (SendBytes <= 0)
				{
					std::cerr << "\nFailed to send file: " << FileName << std::endl;
					SendingFile.close();
					delete[] MessageBuf;
					return false;
				}

				// Add sending bytes
				SendMessageSize += SendBytes;
				memset(MessageBuf, 0, SendFileBufLen);
			}

			// End while loop skipping
		SendLittleFile:

			SendingFile.read(MessageBuf, SendFileBufLen);
			SendBytes = send(FileSendSocket, MessageBuf, SendFileBufLen, 0);

			if (SendBytes <= 0)
			{
				std::cerr << "\nFailed to send file: " << FileName << std::endl;
				SendingFile.close();
				delete[] MessageBuf;
				return false;
			}

			if (ProgressFunction != nullptr)
				ProgressFunction(FileSize, FileSize, 0, 0);

			SendingFile.close();
		}

		else
		{
			std::cerr << "\nFailed to open file: " << FileName << std::endl;
			delete[] MessageBuf;
			return false;
		}

		delete[] MessageBuf;
		return true;
	}

	bool RecvFile(EN_SOCKET FileSendSocket, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		// Get file name and file size
		std::string FileInfo;
		if (EN::TCP_Recv(FileSendSocket, FileInfo) == false)
		{
			std::cerr << "\nFailed to received file name" << std::endl;
			return false;
		}
		

		std::string FileName = Split(FileInfo)[0];
		uint64_t FileSize = std::stoll(Split(FileInfo)[1]);

		// Create received file
		if (IsFileExist(FileName))
		{
			std::string Name = FileName.substr(0, FileName.rfind("."));
			std::string Type = FileName.substr(FileName.rfind("."));
			int i = 1;
			while (IsFileExist(Name + " (" + std::to_string(i) + ")" + Type))
				i += 1;
			FileName = Name + " (" + std::to_string(i) + ")" + Type;
		}
		std::ofstream ReceivedFile(FileName, std::ios::binary | std::ios::trunc);


		// Already received bytes
		uint64_t ReceivedMessageSize = 0;

		// Received file buffer 
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
					delete[] MessageBuf;
					return false;
				}

				if (ProgressFunction != nullptr && std::time(0) > t)
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
					delete[] MessageBuf;
					return false;
				}

				ReceivedMessageSize += ReceiveBytes;
				ReceivedFile.write(MessageBuf, SendFileBufLen);
				memset(MessageBuf, 0, SendFileBufLen);
			}

			// End skipping while loop
		RecvLittleFile:
			int ReceiveBytes = recv(FileSendSocket, MessageBuf, SendFileBufLen, MSG_WAITALL);

			if (ReceiveBytes <= 0)
			{
				std::cerr << "\nFailed to received file: " << FileName << std::endl;
				ReceivedFile.close();
				delete[] MessageBuf;
				return false;
			}
			
			ReceivedFile.write(MessageBuf, FileSize % SendFileBufLen);

			if (ProgressFunction != nullptr)
				ProgressFunction(FileSize, FileSize, 0, 0);

			ReceivedFile.close();
		}
		else
		{
			std::cerr << "\nFailed to send file: " << FileName << std::endl;
			delete[] MessageBuf;
			return false;
		}
		
		delete[] MessageBuf;
		return true;
	}

	bool ContinueRecvFile(EN_SOCKET FileSendSocket, bool& IsStop, void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		// Get file name and file size
		std::string FileInfo;
		if (EN::TCP_Recv(FileSendSocket, FileInfo) == false)
		{
			std::cerr << "\nFailed to received file name" << std::endl;
			return false;
		}

		std::string FileName = Split(FileInfo)[0];
		uint64_t FileSize = std::stoll(Split(FileInfo)[1]);

		if (!IsFileExist(FileName))
		{
			std::cerr << "No file to re-receive!" << std::endl;
			return false;
		}

		std::ofstream ReceivedFile(FileName, std::ios::app | std::ios::binary);
	
		// Already received bytes
		uint64_t ReceivedMessageSize = 0;

		// Received file buffer 
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
					delete[] MessageBuf;
					return false;
				}

				if (ProgressFunction != nullptr && std::time(0) > t)
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
					delete[] MessageBuf;
					return false;
				}

				ReceivedMessageSize += ReceiveBytes;
				ReceivedFile.write(MessageBuf, SendFileBufLen);
				memset(MessageBuf, 0, SendFileBufLen);
			}

			// End skipping while loop
		RecvLittleFile:
			int ReceiveBytes = recv(FileSendSocket, MessageBuf, SendFileBufLen, MSG_WAITALL);

			if (ReceiveBytes <= 0)
			{
				std::cerr << "\nFailed to received file: " << FileName << std::endl;
				ReceivedFile.close();
				delete[] MessageBuf;
				return false;
			}

			ReceivedFile.write(MessageBuf, FileSize % SendFileBufLen);

			if (ProgressFunction != nullptr)
				ProgressFunction(FileSize, FileSize, 0, 0);

			ReceivedFile.close();
		}
		else
		{
			std::cerr << "\nFailed to send file: " << FileName << std::endl;
			delete[] MessageBuf;
			return false;
		}

		delete[] MessageBuf;
		return true;
	}

	bool ForwardFile(EN_SOCKET SourceFileSocket, EN_SOCKET DestinationFileSocket, bool& IsStop, void(*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta))
	{
		// Get file name and file size
		std::string FileInfo;
		if (EN::TCP_Recv(SourceFileSocket, FileInfo) == false)
		{
			std::cerr << "\nFailed to received file name" << std::endl;
			return false;
		}
		
		EN::TCP_Send(DestinationFileSocket, FileInfo);

		uint64_t FileSize = std::stoll(Split(FileInfo)[1]);

		// Already received bytes
		uint64_t ReceivedMessageSize = 0;

		// Received file buffer 
		char* MessageBuf = new char[SendFileBufLen];

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
				delete[] MessageBuf;
				return false;
			}

			if (ProgressFunction != nullptr && std::time(0) > t)
			{
				ProgressFunction(ReceivedMessageSize, FileSize, ReceivedMessageSize - LastReceivedMessageSize, (FileSize - ReceivedMessageSize) / (ReceivedMessageSize - LastReceivedMessageSize));
				LastReceivedMessageSize = ReceivedMessageSize;
				t = std::time(0);
			}

			int ReceiveBytes = recv(SourceFileSocket, MessageBuf, SendFileBufLen, MSG_WAITALL);

			if (ReceiveBytes <= 0)
			{
				std::cerr << "\nFailed to forward file" << std::endl;

				delete[] MessageBuf;
				return false;
			}

			// Send data to client
			int SendBytes = send(DestinationFileSocket, MessageBuf, SendFileBufLen, 0);

			if (SendBytes <= 0)
			{
				std::cerr << "\nFailed to forward file" << std::endl;
				delete[] MessageBuf;
				return false;
			}

			ReceivedMessageSize += SendBytes;
			memset(MessageBuf, 0, SendFileBufLen);
		}

	// End skipping while loop
	RecvLittleFile:

		int ReceiveBytes = recv(SourceFileSocket, MessageBuf, SendFileBufLen, MSG_WAITALL);

		if (ReceiveBytes <= 0)
		{
			std::cerr << "\nFailed to forward file" << std::endl;
			delete[] MessageBuf;
			return false;
		}

		// Send data to client
		int SendBytes = send(DestinationFileSocket, MessageBuf, SendFileBufLen, 0);

		if (SendBytes <= 0)
		{
			std::cerr << "\nFailed to forward file" << std::endl;
			delete[] MessageBuf;
			return false;
		}

		if (ProgressFunction != nullptr)
			ProgressFunction(FileSize, FileSize, 0, 0);		

		delete[] MessageBuf;
		return true;
	}

	void DownloadStatus(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta)
	{
		if (all <= 0)
			return;

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
			Eta = std::to_string(eta / 3600) + " h " + std::to_string((eta % 3600) / 60) + " m";
		}

		std::cout << "\rDownloaded: " << current / AllMeasureSize << " " << AllMeasureName << " All: " << all / AllMeasureSize
			<< " " << AllMeasureName << " " << (100 * current / all) << " % " << " Speed : " << speed / SpeedSize << " " << SpeedName
			<< " ETA : " << Eta << "                     " << std::flush;

		if (current == all)
			std::cout << "\nEnd file transfering" << std::endl;
	}

	bool IsFileExist(std::string FilePath)
	{
		bool isExist = false;
		std::ifstream fin(FilePath.c_str());

		if (fin.is_open())
			isExist = true;

		fin.close();
		return isExist;
	}

	uint64_t GetFileSize(std::string FileName)
	{
		std::ifstream file(FileName, std::ios::ate | std::ios::binary);
		uint64_t FileSize = 0;

		if (!file.is_open())
		{
			std::cerr << "Couldn't open the file" << std::endl;
		}
		else
			FileSize = file.tellg();

		file.close();
		return FileSize;
	}

	void Delay(int milliseconds)
	{
		#if defined WIN32 || defined _WIN64
		Sleep(milliseconds);
		#else
		usleep(milliseconds * 1000);
		#endif
	}
}