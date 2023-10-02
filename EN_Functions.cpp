#include "EN_Functions.h"

namespace EN
{
	std::size_t MaxTcpMessageSize = 1073741824;
	std::size_t MaxUdpMessageSize = 16384;

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

	void SetMaxTcpMessageSize(std::size_t maxTcpMessageSize)
	{
		MaxTcpMessageSize = maxTcpMessageSize;
	}

	std::size_t GetMaxTcpMessageSize()
	{
		return MaxTcpMessageSize;
	}

	void SetMaxUdpMessageSize(std::size_t maxUdpMessageSize)
	{
		MaxUdpMessageSize = maxUdpMessageSize;
	}

	std::size_t GetMaxUdpMessageSize()
	{
		return MaxUdpMessageSize;
	}

	bool Default_TCP_Send(EN_SOCKET sock, const std::string& message)
	{
		// Sending data prefix with information about message length
		std::string messageLengthString;
		std::size_t messageLength = message.length();

		if (messageLength > MaxTcpMessageSize)
		{
			LOG(LogLevels::Warning, "Trying to send message longer then maximal message size");
			return false;
		}

		if (messageLength == 0) messageLengthString = '\0';
		
		// Convert message length to 128 base system
		while(messageLength > 0)
		{
			// Get last seven bits. Equals to % 128
			messageLengthString += (unsigned char)(messageLength & 0b01111111);
			messageLength >>= 7;
		}

		messageLength = message.length();

		// Alloc memory to sending message
		unsigned char* msgBuf = new unsigned char[messageLengthString.length() + messageLength];

		// Set first bit in length bytes except last length byte
		for (std::size_t i = 0; i < messageLengthString.length() - 1; ++i)
			msgBuf[i] = ((unsigned char)messageLengthString[i]) | 0b10000000;
		
		msgBuf[messageLengthString.length() - 1] = messageLengthString[messageLengthString.length() - 1];

		// Fill sending buffer with data from string
		std::size_t counter = messageLengthString.length();
		for (auto& it : message)
		{
			msgBuf[counter] = (unsigned char)it;
			++counter;
		}

		// Send all data in one send call
		int sendedBytes = send(sock, (char*)msgBuf, messageLengthString.length() + messageLength, 0);
		delete[] msgBuf;

		return ((size_t)sendedBytes == messageLengthString.length() + messageLength);
	}

	bool Default_TCP_Recv(EN_SOCKET sock, std::string& message)
	{
		std::string messageSizeString;
		int receivedBytes;

		// We read the incoming data one byte at a time until we get the first byte with the first bit equal to 0, 
		// which means that this is the last byte with the size.
		while (true)
		{
			char sizeByte;
			receivedBytes = recv(sock, &sizeByte, 1, MSG_WAITALL);

			if (receivedBytes < 1)
			{
				message = "";
				CloseSocket(sock);
				return false;
			}

			messageSizeString += sizeByte;
			if ((sizeByte & 0b10000000) == 0)
				break;
		}

		std::size_t messageSize = 0;

		// Calculate message size
        for (auto it = messageSizeString.rbegin(); it != --messageSizeString.rend(); ++it)
        {
            messageSize += (unsigned char)*it & 0b01111111;
            messageSize *= 128;
        }

        messageSize += (unsigned char)messageSizeString[0] & 0b01111111;

		if (messageSize != 0)
		{
			if (messageSize > MaxTcpMessageSize)
			{
				LOG(LogLevels::Warning, "Trying to recv message longer then maximal message size");
				return false;
			}

			// Allocate memory to incoming message
			char* msg = new char[messageSize];

			receivedBytes = recv(sock, msg, messageSize, MSG_WAITALL);

			if ((std::size_t)receivedBytes != messageSize)
			{
				message = "";
				CloseSocket(sock);
				delete[] msg;
				return false;
			}

	    	message.clear();
	    	for (std::size_t i = 0; i < messageSize; ++i)
			{
		    	message += msg[i];
			}
			
			delete[] msg;
		}
		else
			message.clear();

		return true;
	}

	void Default_UDP_Send(EN_SOCKET sock, std::string destinationAddress, const std::string& message)
	{
		auto SplittedAddr = Split(destinationAddress, ":");
		sockaddr_in ClientAddr;
		//Prepare the sockaddr_in structure
		ClientAddr.sin_family = AF_INET;

		int port;
		if (!StringToInt(SplittedAddr[1], port))
			return;

		ClientAddr.sin_port = htons(port);
		// Set ip address
		inet_pton(AF_INET, SplittedAddr[0].c_str(), &ClientAddr.sin_addr);

		size_t messageLength = message.length();
		if (messageLength > MaxUdpMessageSize)
		{
			LOG(LogLevels::Warning, "Trying to recv message longer then maximal message size");
			return;
		}

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

	bool Default_UDP_Recv(EN_SOCKET sock, std::string& sourceAddress, std::string& message)
	{
		// Source address
		sockaddr_in sourceSockAddr;
		int sizeofaddr = sizeof(sourceSockAddr);
		int receivedBytes;

		char* msg = new char[MaxUdpMessageSize];

		#if defined WIN32 || defined _WIN64
		//try to receive some data, this is a blocking call
		receivedBytes = recvfrom(sock, msg, MaxUdpMessageSize, 0, (sockaddr*)&sourceSockAddr, &sizeofaddr);
		#else
		//try to receive some data, this is a blocking call
		receivedBytes = recvfrom(sock, &msg, MaxUdpMessageSize, 0, (sockaddr*)&sourceSockAddr, (socklen_t*)&sizeofaddr);
		#endif

		if (receivedBytes <= 0)
		{
			message = "";
			CloseSocket(sock);
			delete[] msg;
			return false;
		}

		int msg_size, messageLengthInBytes = 1;
		unsigned char firstMessageLengthByte = msg[0];

		// If first bit equls 1, then message length store in 2 bytes and we need to read next byte
		if (firstMessageLengthByte & 0b10000000)
		{
			unsigned char secondMessageLengthByte;
			receivedBytes = recv(sock, (char*)&secondMessageLengthByte, 1, MSG_WAITALL);

			if (receivedBytes < 1)
			{
				message = "";
				CloseSocket(sock);
				delete[] msg;
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
			delete[] msg;
			return true;
		}

		char str[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &(sourceSockAddr.sin_addr), str, INET_ADDRSTRLEN);
		sourceAddress = std::string(str) + ":" + std::to_string(ntohs(sourceSockAddr.sin_port));

	    message.clear();
	    for (int i = 0; i < msg_size; ++i)
		    message += msg[i + messageLengthInBytes];
		
		delete[] msg;
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

	std::vector<std::string> Split(const std::string& stringToSplit, const std::string& splitterString)
	{
		std::vector<std::string> returnVector;
		size_t i = 0;
		std::string splittedString = "";
		while (i < stringToSplit.size())
		{
			if (stringToSplit[i] == splitterString[0])
			{
				bool isSplitter = true;
				for (size_t j = 1; j < splitterString.size(); ++j)
				{
					if (stringToSplit[i + j] != splitterString[j])
					{
						isSplitter = false;
						break;
					}
				}
				if (isSplitter)
				{
					returnVector.push_back(splittedString);
					splittedString = "";
					i += splitterString.size();
					continue;
				}
			}
			splittedString += stringToSplit[i];
			++i;
		}
		returnVector.push_back(splittedString);
		return returnVector;
	}

	std::vector<std::size_t> FindAllOccurrences(const std::string& stringToFindIn, const std::string& splitterString)
	{
		std::vector<std::size_t> returnVector;
		size_t i = 0;
		while (i < stringToFindIn.size())
		{
			if (stringToFindIn[i] == splitterString[0])
			{
				bool isSplitter = true;
				for (size_t j = 1; j < splitterString.size(); ++j)
				{
					if (stringToFindIn[i + j] != splitterString[j])
					{
						isSplitter = false;
						break;
					}
				}
				if (isSplitter)
				{
					returnVector.push_back(i);
					i += splitterString.size();
					continue;
				}
			}
			++i;
		}
		return returnVector;
	}

	bool SendFile(EN_SOCKET fileSendSocket, std::string fileName, std::atomic_bool& isStop, std::atomic_int& microsecondsBetweenSendingChunks,
		uint64_t previouslySendedSize, EN_FileTransmissionStatus& fileTransmissionStatus)
	{
		// Open sending file
		std::ifstream sendingFile(fileName, std::ios::binary);

		if (!sendingFile.is_open())
		{
            LOG(Error, "Failed to open sending file. File name: " + fileName);
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);			
			return false;
		}

		// Char array to keep file chunks
		char* messageBuf = new char[SendFileBufLen];

		// Get file size
		sendingFile.seekg(0, std::ios::end);
		uint64_t fileSize = sendingFile.tellg();

		// Send file name, previosly sended size and full file size
		if (!EN::Default_TCP_Send(fileSendSocket, fileName + " " + std::to_string(previouslySendedSize) + " "+ std::to_string(fileSize - previouslySendedSize)))
		{
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		// We are waiting for confirmation from the receiving party about the possibility to send accept
		std::string isRecvAvailable;
		if (!EN::Default_TCP_Recv(fileSendSocket, isRecvAvailable) || isRecvAvailable == "bad")
		{
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			LOG(LogLevels::Error, "Receiving party cannot open file!");
			return false;
		}

		// See file start
		sendingFile.seekg(0, std::ios::beg);

		// Current iteration sended bytes
		int sendedBytes;

		// Amount of sended data previously
		sendingFile.seekg(previouslySendedSize, std::ios::beg);

		// Sended bytes
		uint64_t sendedMessageSize = previouslySendedSize;

		EN_BackgroundTimer timer;

		// Set static file transmission variables
		fileTransmissionStatus.SetFileSize(fileSize);

		if (sendingFile.is_open())
		{
			// Variable to store how much bytes was sended before
			uint64_t lastSendMessageSize = 0;

			// Skip while loop if file size less when send buffer
			if (fileSize < SendFileBufLen)
				goto SendLittleFile;

			while (sendedMessageSize < fileSize - SendFileBufLen)
			{
				if (isStop.load() == true)
				{
                    sendingFile.close();
					delete[] messageBuf;
					fileTransmissionStatus.SetIsTransmissionSucceed(false);
					fileTransmissionStatus.SetIsTransmissionEnded(true);
					return false;
				}

				// Regulate transfering speed
				if (microsecondsBetweenSendingChunks.load() > 0)
					EN::Delay<std::chrono::microseconds>(microsecondsBetweenSendingChunks.load());

				// Read binary data
				sendingFile.read(messageBuf, SendFileBufLen);
				// Send data to server
				sendedBytes = send(fileSendSocket, messageBuf, SendFileBufLen, 0);

				if (sendedBytes != SendFileBufLen)
				{
                    LOG(Error, "Failed to send file: " + fileName);
					sendingFile.close();
					delete[] messageBuf;
					fileTransmissionStatus.SetIsTransmissionSucceed(false);
					fileTransmissionStatus.SetIsTransmissionEnded(true);
					return false;
				}

				// Add sending bytes
				sendedMessageSize += sendedBytes;

				// Call progress function
				if (!timer.IsSleep())
				{
					timer.StartTimer<std::chrono::seconds>(1);
					fileTransmissionStatus.SetTransferedBytes(sendedMessageSize);
					fileTransmissionStatus.SetTransmissionSpeed(sendedMessageSize - lastSendMessageSize);
					if (sendedMessageSize - lastSendMessageSize != 0)
						fileTransmissionStatus.SetTransmissionEta((fileSize - sendedMessageSize) / (sendedMessageSize - lastSendMessageSize));

					if (fileTransmissionStatus.GetIsSetProgressFunction())
						fileTransmissionStatus.InvokeProgressFunction();

					lastSendMessageSize = sendedMessageSize;
				}
			}

			// End while loop skipping
		SendLittleFile:

			sendingFile.read(messageBuf, SendFileBufLen);
			sendedBytes = send(fileSendSocket, messageBuf, SendFileBufLen, 0);

			if (sendedBytes != SendFileBufLen)
			{
                LOG(Error, "Failed to send file: " + fileName);
				sendingFile.close();
				delete[] messageBuf;
				fileTransmissionStatus.SetIsTransmissionSucceed(false);
				fileTransmissionStatus.SetIsTransmissionEnded(true);
				return false;
			}

			fileTransmissionStatus.SetTransferedBytes(fileSize);
			fileTransmissionStatus.SetTransmissionSpeed(0);
			fileTransmissionStatus.SetTransmissionEta(0);

			if (fileTransmissionStatus.GetIsSetProgressFunction())
				fileTransmissionStatus.InvokeProgressFunction();

			sendingFile.close();
		}
		else
		{
            LOG(Error, "Failed to open file: " + fileName);
			delete[] messageBuf;
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		delete[] messageBuf;
		fileTransmissionStatus.SetIsTransmissionSucceed(true);
		fileTransmissionStatus.SetIsTransmissionEnded(true);
		return true;
	}

	bool RecvFile(EN_SOCKET FileSendSocket, std::atomic_bool& IsStop, EN_FileTransmissionStatus& fileTransmissionStatus)
	{
		// Get file name and file size
		std::string fileInfo;
		if (EN::Default_TCP_Recv(FileSendSocket, fileInfo) == false)
		{
            LOG(Error, "Failed to received file name");
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		std::vector<std::string> fileInfos = Split(fileInfo);
		std::string fileName = fileInfos[0];
		uint64_t fileSize;
		uint64_t previoslySendedBytes;

		if (!StringToUnsignedLongLong(fileInfos[1], previoslySendedBytes) || !StringToUnsignedLongLong(fileInfos[2], fileSize))
		{
			EN::Default_TCP_Send(FileSendSocket, "bad");
			LOG(Error, "Failed to received file size. Message \"" + fileInfo + "\" not corrected");
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		// Open received file
		std::ofstream receivedFile(fileName + ".tmp", std::ios::binary | std::ios::app);

		// Already received bytes
		uint64_t receivedMessageSize = 0;

		// Received file buffer
		char* messageBuf = new char[SendFileBufLen];
		int receivedBytes;

		EN_BackgroundTimer timer;

		// Set static file transmission variables
		fileTransmissionStatus.SetFileSize(fileSize);

		if (receivedFile.is_open())
		{
			EN::Default_TCP_Send(FileSendSocket, "ok");
			uint64_t lastReceivedMessageSize = 0;

			// Skip while loop if file size less when send buffer
			if (fileSize < SendFileBufLen)
				goto RecvLittleFile;

			while (receivedMessageSize < fileSize - SendFileBufLen)
			{
				if (IsStop.load() == true)
				{
					IsStop.store(false);
					receivedFile.close();
					delete[] messageBuf;
					fileTransmissionStatus.SetIsTransmissionSucceed(false);
					fileTransmissionStatus.SetIsTransmissionEnded(true);
					return false;
				}

				receivedBytes = recv(FileSendSocket, messageBuf, SendFileBufLen, MSG_WAITALL);

				if (receivedBytes != SendFileBufLen)
				{
                    LOG(Error, "Failed to received file: " + fileName);
					receivedFile.close();
					delete[] messageBuf;
					fileTransmissionStatus.SetIsTransmissionSucceed(false);
					fileTransmissionStatus.SetIsTransmissionEnded(true);
					return false;
				}

				receivedMessageSize += receivedBytes;
				receivedFile.write(messageBuf, SendFileBufLen);

				// Call progress function
				if (!timer.IsSleep())
				{
					timer.StartTimer<std::chrono::seconds>(1);
					fileTransmissionStatus.SetTransferedBytes(previoslySendedBytes + receivedMessageSize);
					fileTransmissionStatus.SetTransmissionSpeed(receivedMessageSize - lastReceivedMessageSize);
					if (receivedMessageSize - lastReceivedMessageSize != 0)
						fileTransmissionStatus.SetTransmissionEta((fileSize - receivedMessageSize) / (receivedMessageSize - lastReceivedMessageSize));

					if (fileTransmissionStatus.GetIsSetProgressFunction())
						fileTransmissionStatus.InvokeProgressFunction();

					lastReceivedMessageSize = receivedMessageSize;
				}
			}

			// End skipping while loop
		RecvLittleFile:
			receivedBytes = recv(FileSendSocket, messageBuf, SendFileBufLen, MSG_WAITALL);

			if (receivedBytes != SendFileBufLen)
			{
                LOG(Error, "Failed to received file: " + fileName);
				receivedFile.close();
				delete[] messageBuf;
				fileTransmissionStatus.SetIsTransmissionSucceed(false);
				fileTransmissionStatus.SetIsTransmissionEnded(true);
				return false;
			}

			receivedFile.write(messageBuf, fileSize - receivedMessageSize);

			fileTransmissionStatus.SetTransferedBytes(fileSize);
			fileTransmissionStatus.SetTransmissionSpeed(0);
			fileTransmissionStatus.SetTransmissionEta(0);

			if (fileTransmissionStatus.GetIsSetProgressFunction())
				fileTransmissionStatus.InvokeProgressFunction();

			receivedFile.close();
		}
		else
		{
			EN::Default_TCP_Send(FileSendSocket, "bad");
            LOG(Error, "Failed to received file: " + fileName);
			delete[] messageBuf;
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		// Check if it was file with same name
		std::string newFileName = fileName;
		if (IsFileExist(fileName))
		{
			std::string name = fileName.substr(0, fileName.rfind("."));
			std::string type = fileName.substr(fileName.rfind("."));
			int i = 1;
			while (IsFileExist(name + " (" + std::to_string(i) + ")" + type))
				i += 1;
			newFileName = name + " (" + std::to_string(i) + ")" + type;
		}
		rename((fileName + ".tmp").c_str(), newFileName.c_str());

		delete[] messageBuf;
		fileTransmissionStatus.SetIsTransmissionSucceed(true);
		fileTransmissionStatus.SetIsTransmissionEnded(true);
		return true;
	}

	bool ForwardFile(EN_SOCKET SourceFileSocket, EN_SOCKET DestinationFileSocket, std::atomic_bool& IsStop, EN_FileTransmissionStatus& fileTransmissionStatus)
	{
		// Get file name and file size
		std::string fileInfo;
		if (EN::Default_TCP_Recv(SourceFileSocket, fileInfo) == false)
		{
            LOG(Error, "Failed to received file name");
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		if (!EN::Default_TCP_Send(DestinationFileSocket, fileInfo))
		{
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		uint64_t fileSize;

		if (!StringToUnsignedLongLong(Split(fileInfo)[1], fileSize))
		{
			LOG(Error, "Failed to received file size. Message \"" + fileInfo + "\" not corrected");
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		// Already received bytes
		uint64_t receivedMessageSize = 0;

		// Received file buffer
		char* messageBuf = new char[SendFileBufLen];

		uint64_t lastReceivedMessageSize = 0;
		int receivedBytes, sendedBytes;

		EN_BackgroundTimer timer;

		// Set static file transmission variables
		fileTransmissionStatus.SetFileSize(fileSize);

		// Skip while loop if file size less when send buffer
		if (fileSize < SendFileBufLen)
			goto RecvLittleFile;

		while (receivedMessageSize < fileSize - SendFileBufLen)
		{
			if (IsStop.load() == true)
			{
				IsStop.store(false);
				delete[] messageBuf;
				fileTransmissionStatus.SetIsTransmissionSucceed(false);
				fileTransmissionStatus.SetIsTransmissionEnded(true);
				return false;
			}

			receivedBytes = recv(SourceFileSocket, messageBuf, SendFileBufLen, MSG_WAITALL);

			if (receivedBytes != SendFileBufLen)
			{
                LOG(Error, "Failed to forward file");
				delete[] messageBuf;
				fileTransmissionStatus.SetIsTransmissionSucceed(false);
				fileTransmissionStatus.SetIsTransmissionEnded(true);
				return false;
			}

			// Send data to client
			sendedBytes = send(DestinationFileSocket, messageBuf, SendFileBufLen, 0);

			if (sendedBytes != SendFileBufLen)
			{
				LOG(Error, "Failed to forward file");
				delete[] messageBuf;
				fileTransmissionStatus.SetIsTransmissionSucceed(false);
				fileTransmissionStatus.SetIsTransmissionEnded(true);
				return false;
			}

			receivedMessageSize += sendedBytes;

			// Call progress function
			if (!timer.IsSleep())
			{
				timer.StartTimer<std::chrono::seconds>(1);
				fileTransmissionStatus.SetTransferedBytes(receivedMessageSize);
				fileTransmissionStatus.SetTransmissionSpeed(receivedMessageSize - lastReceivedMessageSize);
				if (receivedMessageSize - lastReceivedMessageSize != 0)
					fileTransmissionStatus.SetTransmissionEta((fileSize - receivedMessageSize) / (receivedMessageSize - lastReceivedMessageSize));

				if (fileTransmissionStatus.GetIsSetProgressFunction())
					fileTransmissionStatus.InvokeProgressFunction();

				lastReceivedMessageSize = receivedMessageSize;
			}
		}

	// End skipping while loop
	RecvLittleFile:

		receivedBytes = recv(SourceFileSocket, messageBuf, SendFileBufLen, MSG_WAITALL);

		if (receivedBytes != SendFileBufLen)
		{
			LOG(Error, "Failed to forward file");
			delete[] messageBuf;
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		// Send data to client
		sendedBytes = send(DestinationFileSocket, messageBuf, SendFileBufLen, 0);

		if (sendedBytes != SendFileBufLen)
		{
			LOG(Error, "Failed to forward file");
			delete[] messageBuf;
			fileTransmissionStatus.SetIsTransmissionSucceed(false);
			fileTransmissionStatus.SetIsTransmissionEnded(true);
			return false;
		}

		fileTransmissionStatus.SetTransferedBytes(fileSize);
		fileTransmissionStatus.SetTransmissionSpeed(0);
		fileTransmissionStatus.SetTransmissionEta(0);

		if (fileTransmissionStatus.GetIsSetProgressFunction())
			fileTransmissionStatus.InvokeProgressFunction();

		delete[] messageBuf;

		fileTransmissionStatus.SetIsTransmissionSucceed(true);
		fileTransmissionStatus.SetIsTransmissionEnded(true);
		return true;
	}

	void DefaultDownloadStatusFunction(uint64_t transferedBytes, uint64_t fileSize, uint64_t transmissionSpeed, uint64_t transmissionEta)
	{
		if (fileSize <= 0)
			return;

		std::string AllMeasureName, SpeedName, Eta;
		int AllMeasureSize, SpeedSize;

		AllMeasureName = "KB";
		AllMeasureSize = 1024;
		SpeedName = "KB/s";
		SpeedSize = 1024;
		if (fileSize > 1024 * 1024)
		{
			AllMeasureName = "MB";
			AllMeasureSize = 1024 * 1024;
		}
		if (transmissionSpeed > 1024 * 1024)
		{
			SpeedName = "MB/s";
			SpeedSize = 1024 * 1024;
		}

		if (transmissionEta < 60)
		{
			Eta = std::to_string(transmissionEta) + " s";
		}
		if (transmissionEta > 60 && transmissionEta < 3600)
		{
			Eta = std::to_string(transmissionEta / 60) + " m";
		}
		if (transmissionEta > 3600)
		{
			Eta = std::to_string(transmissionEta / 3600) + " h " + std::to_string((transmissionEta % 3600) / 60) + " m";
		}

		std::cout << "\rDownloaded: " << transferedBytes / AllMeasureSize << " " << AllMeasureName << " All: " << fileSize / AllMeasureSize
			<< " " << AllMeasureName << " " << (100 * transferedBytes / fileSize) << " % " << " Speed : " << transmissionSpeed / SpeedSize << " " << SpeedName
			<< " ETA : " << Eta << "                     " << std::flush;

		if (transferedBytes == fileSize)
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

	uint64_t GetFileSize(std::string fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);
		uint64_t fileSize = 0;

		if (!file.is_open())
		{
            LOG(Error, "Couldn't open the file");
            return 0;
		}
		else
			fileSize = file.tellg();

		file.close();
		return fileSize;
	}

    int GetCPUCores()
	{
		return std::thread::hardware_concurrency();
	}

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

	std::string UUID4()
    {
        std::string res;
        std::random_device rd;
        std::uniform_int_distribution<int> dist(0, 15);

        char chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

        for (int i = 0; i < 32; i++)
        {
            int a = dist(rd);

            if (i == 12)
            {
            res += std::to_string(0b0100);
            continue;
            }

            if (i == 16)
            {
                a |= 0b1000;
                a &= ~(0b0100);
            }

            res += chars[a];

            if (i == 7 || i == 11 || i == 15 || i == 19)
                res += "-";
        }

        return res;
    }

	bool IsCanBeDigit(const std::string& str)
    {
        if ((str[0] < '0' && str[0] != '-') || str[0] > '9')
		    return false;

        for (char it : str)
            if (it < '0' || it > '9')
                return false;
        return true;
    }

    bool StringToInt(const std::string& str, int& res)
    {
        if (IsCanBeDigit(str))
        {
            res = std::stoi(str);
            std::cout << res << std::endl;
            return true;
        }
        else
            return false;
    }

    bool StringToLong(const std::string& str, long int& res)
    {
        if (IsCanBeDigit(str))
        {
            res = std::stol(str);
            return true;
        }
        else
            return false;
    }

    bool StringToLongLong(const std::string& str, long long int& res)
    {
        if (IsCanBeDigit(str))
        {
            res = std::stoll(str);
            return true;
        }
        else
            return false;
    }

    bool StringToUnsignedLong(const std::string& str, unsigned long int& res)
    {
        if (IsCanBeDigit(str))
        {
            res = std::stoul(str);
            return true;
        }
        else
            return false;
    }

    bool StringToUnsignedLongLong(const std::string& str, unsigned long long int& res)
    {
        if (IsCanBeDigit(str))
        {
            res = std::stoull(str);
            return true;
        }
        else
            return false;
    }
}
