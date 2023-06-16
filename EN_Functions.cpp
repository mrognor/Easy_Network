#include "EN_Functions.h"

namespace EN
{
	#if defined WIN32 || defined _WIN64

    WSA_Init_Cleanup::WSA_Init_Cleanup()
	{ 
		// WSAStartup
		WSAData wsaData;
		if (WSAStartup(MAKEWORD(2, 1), &wsaData) != 0)
		{
			std::cerr << "Error: Library initialization failure." << std::endl;
			exit(1);
		}
	}

	WSA_Init_Cleanup::~WSA_Init_Cleanup()
	{
		// Clean after wsa
		WSACleanup();
	}

	WSA_Init_Cleanup WSA_IC = WSA_Init_Cleanup();

	#endif
	
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

	bool TCP_Send(EN_SOCKET sock, const std::string& message)
	{
		size_t messageLength = message.length();

		// If message length more than 128, then set first bit to 1, that means that message
		// length stored in 2 bytes
		// If message length less or equal 128, than set first bit to 0, and send only 
		// one byte with message length	

		unsigned char* msgBuf;
		int startIndex;
		if (messageLength >= 128)
		{
			msgBuf = new unsigned char[2 + (int)messageLength];
			msgBuf[0] = (unsigned char)((messageLength / 128) | 0b10000000);
			msgBuf[1] = (unsigned char)(messageLength % 128);
			startIndex = 2;
		}
		else
		{
			msgBuf = new unsigned char[1 + (int)messageLength];
			msgBuf[0] = (unsigned char)messageLength;
			startIndex = 1;
		}	

		for (int i = startIndex; i < (int)messageLength + startIndex; ++i)
			msgBuf[i] = message[i - startIndex];
		
		int sendedBytes = send(sock, (char*)msgBuf, (int)messageLength + startIndex, 0);
		delete[] msgBuf;

		// Return true if sended butes equals message length, otherwise return false
		return (sendedBytes == messageLength);
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

		if (ConnectionStatus != msg_size)
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

	void UDP_Send(EN_SOCKET sock, std::string destinationAddress, const std::string& message)
	{
		auto SplittedAddr = Split(destinationAddress, ":");
		sockaddr_in ClientAddr;
		//Prepare the sockaddr_in structure
		ClientAddr.sin_family = AF_INET;
		ClientAddr.sin_port = htons(std::atoi(SplittedAddr[1].c_str()));
		// Set ip address
		inet_pton(AF_INET, SplittedAddr[0].c_str(), &ClientAddr.sin_addr);

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
			msgBuf[0] = (int)(messageLength / 128);
			msgBuf[0] |= 0b10000000;
			msgBuf[1] = (int)(messageLength % 128);
			messageByteLength = 2;
		}
		else
		{
			msgBuf = new unsigned char[1  + messageLength];
			msgBuf[0] = (int)messageLength;
			messageByteLength = 1;
		}		

		for (size_t i = 0; i < messageLength; ++i)
			msgBuf[i + messageByteLength] = message[i];

		sendto(sock, (char*)msgBuf, messageLength + messageByteLength, 0, (sockaddr*)&ClientAddr, (int)sizeof(ClientAddr));
		
		delete[] msgBuf;
	}

	bool UDP_Recv(EN_SOCKET sock, std::string& sourceAddress, std::string& message)
	{
		// Source address
		sockaddr_in sourceSockAddr;
		int sizeofaddr = sizeof(sourceSockAddr);
		int ConnectionStatus;

		char msg[4096];
		#if defined WIN32 || defined _WIN64
		//try to receive some data, this is a blocking call
		ConnectionStatus = recvfrom(sock, (char*)&msg, 4096, 0, (sockaddr*)&sourceSockAddr, &sizeofaddr);
		#else
		//try to receive some data, this is a blocking call
		ConnectionStatus = recvfrom(sock, (char*)&msg, 4096, 0, (sockaddr*)&sourceSockAddr, (socklen_t*)&sizeofaddr);
		#endif

		if (ConnectionStatus <= 0)
		{
			message = "";

			CloseSocket(sock);
			return false;
		}

		int msg_size, messageLengthInBytes = 1;
		unsigned char firstMessageLengthByte = msg[0];

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
			messageLengthInBytes = 2;
		}
		else
			msg_size = firstMessageLengthByte & 0b01111111;
		
		if (msg_size <= 0)
		{
			message = "";
			return true;
		}

		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(sourceSockAddr.sin_addr), str, INET_ADDRSTRLEN);
		sourceAddress = std::string(str) + ":" + std::to_string(ntohs(sourceSockAddr.sin_port));

	    message.clear();
	    for (int i = 0; i < msg_size; ++i)
		    message += msg[i + messageLengthInBytes];

		return true;
	}

	void CloseSocket(EN_SOCKET sock)
	{
		#if defined WIN32 || defined _WIN64
		closesocket(sock);
		#else
		shutdown(sock, SHUT_RDWR);
		close(sock);
		#endif
	}

	std::vector<std::string> Split(std::string StringToSplit, std::string SplitterString)
	{
		std::vector<std::string> ReturnVector;
		size_t i = 0;
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
		if (!EN::TCP_Send(FileSendSocket, FileName + " " + std::to_string(FileSize - PreviouslySendedSize)))
			return false;

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
                    SendingFile.close();
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
                    LOG(Error, "Failed to send file: " + FileName);
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
                LOG(Error, "Failed to send file: " + FileName);
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
            LOG(Error, "Failed to open file: " + FileName);
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
            LOG(Error, "Failed to received file name");
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
                    LOG(Error, "Failed to received file: " + FileName);
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
                LOG(Error, "Failed to received file: " + FileName);
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
            LOG(Error, "Failed to send file: " + FileName);
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
            LOG(Error, "Failed to received file name");
			return false;
		}

		std::string FileName = Split(FileInfo)[0];
		uint64_t FileSize = std::stoll(Split(FileInfo)[1]);

		if (!IsFileExist(FileName))
		{
            LOG(Error, "No file to re-receive!");
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
                    LOG(Error, "Failed to received file: " + FileName);
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
                LOG(Error, "Failed to received file: " + FileName);
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
            LOG(Error, "Failed to send file: " + FileName);
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
            LOG(Error, "Failed to received file name");
			return false;
		}
		
		if (!EN::TCP_Send(DestinationFileSocket, FileInfo))
			return false;

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
                LOG(Error, "Failed to forward file");
				delete[] MessageBuf;
				return false;
			}

			// Send data to client
			int SendBytes = send(DestinationFileSocket, MessageBuf, SendFileBufLen, 0);

			if (SendBytes <= 0)
			{
				LOG(Error, "Failed to forward file");
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
			LOG(Error, "Failed to forward file");
			delete[] MessageBuf;
			return false;
		}

		// Send data to client
		int SendBytes = send(DestinationFileSocket, MessageBuf, SendFileBufLen, 0);

		if (SendBytes <= 0)
		{
			LOG(Error, "Failed to forward file");
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
		int AllMeasureSize, SpeedSize, EtaSize = 0;

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
            LOG(Error, "Couldn't open the file");
            return 0;
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

    int GetCPUCores() { return std::thread::hardware_concurrency(); }

    int GetSocketErrorCode()
    {
        #if defined WIN32 || defined _WIN64 
            return WSAGetLastError();
        #else
            return (errno);
        #endif
    }

	std::string GetSocketErrorString(int socketErrorCode)
    {
        #if defined WIN32 || defined _WIN64 

        if (socketErrorCode  == -1)
            socketErrorCode = WSAGetLastError();

        char msgBuf [256];
        msgBuf [0] = '\0';
        FormatMessage (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL,
                    socketErrorCode,
                    MAKELANGID (LANG_ENGLISH, SUBLANG_DEFAULT),
                    msgBuf,
                    sizeof (msgBuf),
                    NULL);

        char *nl = strrchr(msgBuf, '\n');
        if (nl) *nl = 0;
        return std::string(msgBuf);

        #else

        if (socketErrorCode  == -1)
            socketErrorCode = errno;
        return strerror(errno);

        #endif
    }

    int GetCurrentSecond()
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        return ltm->tm_sec;
    }

    int GetCurrentMinute()
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        return ltm->tm_min;
    }

    int GetCurrentHour()
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        return ltm->tm_hour;
    }

    int GetCurrentMonthDay()
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        return ltm->tm_mday;
    }

    int GetCurrentYearDay()
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        return (ltm->tm_yday + 1);
    }

    int GetCurrentMonth()
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        return (ltm->tm_mon + 1);
    }

    int GetCurrentYear()
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        return (ltm->tm_year + 1900);
    }

    std::string GetCurrentDayWeek()
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);

        switch(ltm->tm_wday)
        {
        case 0: return "Monday";
        case 1: return "Tuesday";
        case 2: return "Wednesday"; 
        case 3: return "Thursday";
        case 4: return "Friday";
        case 5: return "Saturday";
        case 6: return "Sunday";
        }
        return "";
    }

    std::string GetCurrentDate(bool IsAmericanFormat)
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        std::string currentDate;

        if (IsAmericanFormat)
        {
            if (ltm->tm_mon < 9) currentDate += "0";
            currentDate += std::to_string(ltm->tm_mon + 1);
            currentDate += ".";

            if (ltm->tm_mday < 10) currentDate += "0";
            currentDate += std::to_string(ltm->tm_mday);
            currentDate += ".";
        }
        else
        {
            if (ltm->tm_mday < 10) currentDate += "0";
            currentDate += std::to_string(ltm->tm_mday);
            currentDate += ".";

            if (ltm->tm_mon < 9) currentDate += "0";
            currentDate += std::to_string(ltm->tm_mon + 1);
            currentDate += ".";
        }

        currentDate += std::to_string(1900 + ltm->tm_year);

        return currentDate;
    }

    std::string GetCurrentDayTime()
    {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        std::string currentTime;

        if (ltm->tm_hour < 10) currentTime += "0";
        currentTime += std::to_string(ltm->tm_hour);
        currentTime += ".";
        if (ltm->tm_min < 10) currentTime += "0";
        currentTime += std::to_string(ltm->tm_min);
        currentTime += ".";
        if (ltm->tm_sec < 10) currentTime += "0";
        currentTime += std::to_string(ltm->tm_sec);
        
        return currentTime;
    }

    std::string GetLocalTime()
    {
        time_t now = time(0);
        std::string res(ctime(&now));
        res.pop_back();
        return res;
    }

    std::string GetUTCTime()
    {
        time_t now = time(0);
        tm *gmtm = gmtime(&now);
        char *dt = asctime(gmtm);
        return std::string(dt);
    }

	void SetSocketOption(EN_SOCKET socket, int level, int optionName, int optionValue)
	{
		if (setsockopt(socket, level, optionName, (const char*)&optionValue, sizeof(optionValue)) != 0)
			LOG(EN::LogLevels::Warning, "Invalid socket option! Socket option name: "
				+ std::to_string(optionName)
				+ " Socket option value: "
				+ std::to_string(optionValue)
				+ " Error code: " + std::to_string(GetSocketErrorCode()) + " " + EN::GetSocketErrorString());
	}
}