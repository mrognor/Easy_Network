/** @file */
/// \cond HIDDEN_SYMBOLS

#pragma once

#if defined WIN32 || defined _WIN64 

#include <winsock2.h>
#include <WS2tcpip.h>
typedef SOCKET EN_SOCKET;
#define GETSOCKETERRNO() (WSAGetLastError())

#else

#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef int EN_SOCKET;
#define GETSOCKETERRNO() (errno)

#endif

#include <math.h>
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <fstream>

#include "EN_ParallelFor.h"
#include "EN_Logger.h"

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
	void TCP_Send(EN_SOCKET sock, const std::string& message, int MessageDelay = 10);

	/*!
		Wrapper over the recv function. Allows you to recv std::string. 
		\param[in] sock socket to get data
		\param[out] message string to put received data
		\return Returns true in case of success, false if it was disconnection 
	*/ 
	bool TCP_Recv(EN_SOCKET sock, std::string& message);

	/*!
		Wrapper over the sendto function. Allows you to send std::string. 
		\param[in] sock socket to send data
		\param[in] ipAddress destination ip address and port
		\param[in] message string to put received data
		\param[in] MessageDelay delay after message sending
		\return Returns true in case of success, false if it was disconnection 
	*/ 
	void UDP_Send(EN_SOCKET sock, std::string destinationAddress, const std::string& message, int MessageDelay);

	/*!
		Wrapper over the recv function. Allows you to recv std::string. 
		\param[in] sock socket to get data
		\param[in] ipAddress source ip address and port
		\param[out] message string to put received data
		\param[in] MessageDelay delay after message sending
		\return Returns true in case of success, false if it was disconnection 
	*/ 
	bool UDP_Recv(EN_SOCKET sock, std::string& sourceAddress, std::string& message);

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

	/// \brief Returns the number of processor cores
	int GetCPUCores();

    /** 
        Return string description to socket error
        \param [in] socketErrorCode Socket error code. Optional parameter.
        If you do not specify it, the function itself will take the last socket error from the operating system
    */ 
    std::string GetSocketError(int socketErrorCode = -1);
    
    /// Return current second
    int GetCurrentSecond();

    /// Return current minute
    int GetCurrentMinute();

    /// Return current hour
    int GetCurrentHour();

    /// Return the current day of the month
    int GetCurrentMonthDay();

    /// Return the current day of the year
    int GetCurrentYearDay();

    /// Return current month
    int GetCurrentMonth();

    /// Return current year
    int GetCurrentYear();

    /** \brief Function for getting the current date
        \param [in] IsAmericanFormat A variable for specifying the format of the returned date. By default, the parameter is false
        If the parameter is empty or false, the function returns the date in normal format: dd.mm.yyyy.
        Otherwise it will return in the american format: mm.dd.yyyy.
        \return Return current date in string with format: "day_week_name date" where day_week_name only 3 first char, first char is Capital like Mon or Sun,
        date is date in format dd.mm.yyyy or mm.dd.yyyy depends on input param
    */  
    std::string GetCurrentDate(bool IsAmericanFormat = false);

    /// Return current day time in format: hh.mm.ss
    std::string GetCurrentDayTime();

    /** 
        \brief Return current local time in ctime format
        \return Return current local day time in string with format: "week_day_name month_name day_in_month local_time year" 
        where day_week_name only 3 first char, first char is Capital like Tue or Sat,
        month_name only 3 first char, first char is Capital like May or Mar,
        day_in_month in format: dd like 01 or 12,
        local time in format: hh.mm.ss,
        year in format: yyyy like 2023
    */
    std::string GetLocalTime();

    /** 
        \brief Return current UTC time in ctime format
        \return Return current UTC day time in string with format: "week_day_name month_name day_in_month local_time year" 
        where day_week_name only 3 first char, first char is Capital like Tue or Sat,
        month_name only 3 first char, first char is Capital like May or Mar,
        day_in_month in format: dd like 01 or 12,
        local time in format: hh.mm.ss,
        year in format: yyyy like 2023
    */
    std::string GetUTCTime();

    /**
        This function returns a fraction of a current second. 
        Accepts the time data type from the chrono library as a template.
        
        Possible specializations:
        std::chrono::nanoseconds
        std::chrono::microseconds
        std::chrono::milliseconds

        All other specifications don't make sense
    */
    template <class T>
    int64_t GetCurrentSecondFraction()
    {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::high_resolution_clock::now();
        auto sec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        auto t = std::chrono::duration_cast<T>(now.time_since_epoch()).count();
        return t % sec;
    }

    /**
        This function returns current day time in format: hh.mm.ss.ff. Where f is fraction of a second 
        Accepts the time data type from the chrono library as a template.
        
        Possible specializations:
        std::chrono::nanoseconds  hh.mm.ss.fffffffff
        std::chrono::microseconds hh.mm.ss.ffffff
        std::chrono::milliseconds hh.mm.ss.fff

        All other specifications don't make sense
    */
    template <class T>
    std::string GetCurrentDayTimeWithSecondFraction()
    {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::high_resolution_clock::now();
        time_t cnow = std::chrono::system_clock::to_time_t(now);
        tm* ltm = localtime(&cnow);
        std::string currentTime;

        if (ltm->tm_hour < 10) currentTime += "0";
        currentTime += std::to_string(ltm->tm_hour);
        currentTime += ".";
        if (ltm->tm_min < 10) currentTime += "0";
        currentTime += std::to_string(ltm->tm_min);
        currentTime += ".";
        if (ltm->tm_sec < 10) currentTime += "0";
        currentTime += std::to_string(ltm->tm_sec);

        auto currentTimeSec = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
        auto currentTimeT = std::chrono::duration_cast<T>(now.time_since_epoch()).count();
        std::string stringTimeT = std::to_string(currentTimeT);

        int len = currentTimeT / currentTimeSec;
        switch (len)
        {
        // Milliseconds
        case 1000:
            currentTime = currentTime + "." + stringTimeT.substr(stringTimeT.length() - 3);
            break;
        // Microseconds
        case 1000000:
            currentTime = currentTime + "." + stringTimeT.substr(stringTimeT.length() - 6);
            break;
        // Nanoseconds
        case 1000000000:
            currentTime = currentTime + "." + stringTimeT.substr(stringTimeT.length() - 9);
            break;
        }
        
        return currentTime;
    }

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