/** @file */
/// \cond HIDDEN_SYMBOLS

#pragma once

#if defined WIN32 || defined _WIN64 

#include <winsock2.h>
#include <WS2tcpip.h>
typedef SOCKET EN_SOCKET;

#else

#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int EN_SOCKET;

#endif

#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>

#include "EN_ParallelFor.h"

#define SendFileBufLen 1024
/// \endcond

namespace EN
{
	/// Function for getting an ip address from a url.
	/// \param[in] url address to get ip
	/// \return Return host ip or empty string if cannot get host ip.
	std::string GetIpByURL(std::string url);

	/*!
		Wrapper over the send function. Allows you to send std::string. 
		\param[in] sock socket to send data 
		\param[in] message message to send
		\param[in] MessageDelay delay after message sending
	*/
	void TCP_Send(EN_SOCKET sock, std::string message, int MessageDelay = 10);

	/*!
		Wrapper over the recv function. Allows you to recv std::string. 
		\param[in] socket socket to get data
		\param[out] message string to put received data
		\return Returns true in case of success, false if it was disconnection 
	*/ 
	bool TCP_Recv(EN_SOCKET sock, std::string& message);

	/// Close socket
	void CloseSocket(EN_SOCKET sock);

	/*!
		Divides the string to std::vector<std::string>. 
		\param[in] StringToSplit string to split
		\param[in] SplitterString the string that divides the string. Default set to space(" ")
		\return Vector of strings
	*/ 
	std::vector<std::string> Split(std::string StringToSplit, std::string SplitterString = " ");

	/*! 
		\brief The function gets socket and filename and send file to socket. 
		\param[in] FileSendSocket the socket for sending files
		\param[in] FilePath the full path to the file to be sent
		\param[in] IsStop the reference to a boolean variable to stop file transfer  
		\param[in] ProgressFunction the pointer to a function that is used to track the status of file transfer. By default using EN::DownloadStatus 
		\param[in] PreviouslySendedSize The fifth parameter is needed to continue downloading. Gets the size of the previously transmitted file in bytes. 0 means no previosly sending
		\param[in] ChunksNumberBetweenDelay The sixth parameter is needed to regulate the file transfer rate. Gets amount of chunks between sending delay. 0 means no delay. The delay between sending the chunks is 20 millimeconds.
		\return Returns true in case of file transmition success, false otherwise
		
		By default, the library progress function is used to output to the console.  
		Timeout between sending file chunks set to 20. You adjust the number of chunks that will be sent between the this timeout
	*/ 
	bool SendFile(EN_SOCKET FileSendSocket, std::string FilePath, bool& IsStop, 
		void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr, 
		uint64_t PreviouslySendedSize = 0, int ChunksNumberBetweenDelay = 0);

	/*!
		\brief This function will wait incoming file.
		\param[in] FileSendSocket the socket for receiving files
		\param[in] IsStop the reference to a boolean variable to stop file transfer
		\param[in] ProgressFunction the pointer to a function that is used to track the status of file transfer
		\return Returns true in case of file transmition success, false otherwise
	*/
	bool RecvFile(EN_SOCKET FileSendSocket, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr);

	/*!
		\brief Function to continue receiving the file. This function will wait file.
		\param[in] FileSendSocket the socket for receiving files
		\param[in] IsStop the reference to a boolean variable to stop file transfer
		\param[in] ProgressFunction the pointer to a function that is used to track the status of file transfer
		\return Returns true in case of file transmition success, false otherwise
	*/
	bool ContinueRecvFile(EN_SOCKET FileSendSocket, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr);

	/*!
		\brief This function will forward file from one socket to another.
		\param[in] SourceFileSocket the source socket for receiving files
		\param[in] DestinationFileSocket the destination socket for sending files
		\param[in] IsStop the reference to a boolean variable to stop file transfer
		\param[in] ProgressFunction the pointer to a function that is used to track the status of file transfer
		\return Returns true in case of file transmition success, false otherwise
	*/
	bool ForwardFile(EN_SOCKET SourceFileSocket, EN_SOCKET DestinationFileSocket, bool& IsStop, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta) = nullptr);

	/*!
		\brief This function will print information about file downloading
		\param[in] current the first is how many bytes were transmitted.  
		\param[in] all the second is the total number of bytes.  
		\param[in] speed the third parameter shows the transfer rate in bytes.  
		\param[in] eta the fourth shows the remaining time in seconds
	*/
	void DownloadStatus(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta);

	/// Return true if file exists, otherwise rerurn false
	bool IsFileExist(std::string FilePath);

	/// \brief Return file size in bytes
	/// If couldn't open the file return 0
	uint64_t GetFileSize(std::string FileName);

	/// Crossplatform function for program suspension
	void Delay(int milliseconds);

	/*!
		\brief Function to convert int to string. 
		Works with standart data types. Use this function if your data 
		types support next operators: >>, <<, &

		One char takes 1 byte, and a int takes 4 bytes. 
		If you try to translate the number 120 into a string, then you will have 3 characters or 3 bytes.
		This function turns a number into a character, since a character occupies a byte, 
		then you can transfer numbers up to 255 in one byte. 
		This function can be considered as the translation of a number into a number system with a base of 256
		This function does not work with negative numbers.
	*/
	template <class T>
    std::string IntToString(T n)
    {
        std::string str;

        while(n > 0)
        {
			// Get last eight bits. Equals to % 256
            str += (unsigned char)(n & 0b11111111);
            n >>= 8;
        }

        return str;
    }

	/*!
		\brief Function to convert int to string. 
		Works with custom data types. Use this function if your data 
		types dont support next operators: >>, <<, &

		One char takes 1 byte, and a int takes 4 bytes. 
		If you try to translate the number 120 into a string, then you will have 3 characters or 3 bytes.
		This function turns a number into a character, since a character occupies a byte, 
		then you can transfer numbers up to 255 in one byte. 
		This function can be considered as the translation of a number into a number system with a base of 256
		This function does not work with negative numbers.
	*/
	template <class T>
    std::string WIntToString(T n)
    {
        std::string str;

        while(n > 0)
        {
            str += (unsigned char)(n % T(256));
            n /= 256;
        }

        return str;
    }

	/** \brief Function to convert string to int.
	
		Works with standart data types. Use this function if your data 
		types support next operators: >>, <<, &
		Works with strings from function EN::IntToString.
		\warning Dont forget to specify returning value using <type>. Example: int i = StringToInt<int>("string")
	*/
    template <class T>
    T StringToInt(const std::string& str)
    {
        T n = 0;
        for (auto it = str.rbegin(); it != str.rend(); ++it)
        {
            n += (unsigned char)*it;
            n *= 256;
        }
        n /= 256;
        return n;
    }

	/** \brief Function to convert string to int.
	
		Works with custom data types. Use this function if your data 
		types dont support next operators: >>, <<, &
		Works with strings from function EN::IntToString.
		\warning Dont forget to specify returning value using <type>. Example: int i = WStringToInt<int>("string")
	*/
    template <class T>
    T WStringToInt(const std::string& str)
    {
        T n = 0;
        for (auto it = str.rbegin(); it != str.rend(); ++it)
        {
            n += (unsigned char)*it;
            n <<= 8;
        }
        n >>= 8;
        return n;
    }
}