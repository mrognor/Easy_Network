#pragma once

#ifdef WIN32

#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <WS2tcpip.h>
typedef SOCKET EN_SOCKET;

#else

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int EN_SOCKET;

#endif

#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>

#define SendFileBufLen 1024

namespace EN
{ 
	/// Wrapper over the send function. Allows you to send std::string. 
	void TCP_Send(EN_SOCKET sock, std::string message, int MessageDelay = 10);

	/// Wrapper over the recv function. Allows you to recv std::string. 
	bool TCP_Recv(EN_SOCKET sock, std::string& message);

	/// Divides the string to std::vector<std::string>. Second string is string divider. By default set to space(" ")
	std::vector<std::string> Split(std::string StringToSplit, std::string SplitterString=" ");

	/// Functions gets socket and filename and send file to socket. Third parametr its refernce to bool to stop transmission. 
	/// The fourth parameter is a pointer to a function to handle the remaining transfer time
	/// The fifth parameter is needed to continue downloading. Gets the size of the previously transmitted file in bytes. 0 means no previosly sending
	/// The sixth parameter is needed to regulate the file transfer rate. Gets amount of chunks between sending delay. 0 means no delay
	bool SendFile(EN_SOCKET FileSendSocket, std::string FilePath, bool& IsStop, 
		void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr, 
		uint64_t PreviouslySendedSize = 0, int ChunksNumberBetweenDelay = 0);

	/// This function will wait file. Second parametr its refernce to bool to stop transmission. 
	/// The fird parameter is a pointer to a function to handle the remaining transfer time
	bool RecvFile(EN_SOCKET FileSendSocket, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr);

	/// Function to continue receiving the file
	/// This function will wait file. Second parametr its refernce to bool to stop transmission. 
	/// The fird parameter is a pointer to a function to handle the remaining transfer time
	bool ContinueRecvFile(EN_SOCKET FileSendSocket, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr);

	/// This function will forward file from one socket to another. 
	/// First parametr is source socket
	/// Second parametr is destination socket
	/// Third parametr its refernce to bool to stop transmission. 
	/// The fird parameter is a pointer to a function to handle the remaining transfer time
	bool ForwardFile(EN_SOCKET SourceFileSocket, EN_SOCKET DestinationFileSocket, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr);

	/// This function will print information about file downloading
	void DownloadStatus(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta);

	/// Return true if file exists, otherwise rerurn false
	bool IsFileExist(std::string FilePath);

	/// Return file size in bytes
	/// If couldn't open the file return 0
	uint64_t GetFileSize(std::string FileName);

	/// Crossplatform function for program suspension
	void Delay(int milliseconds);
}