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
#include <exception>
#include <set>
#include <list>
#include <atomic>
#include <random>

#include "EN_ParallelFor.h"
#include "EN_BackgroundTimer.h"
#include "EN_Logger.h"
#include "EN_FileTransmissionStatus.h"

#define SendFileBufLen 4096

namespace EN
{
    #if defined WIN32 || defined _WIN64
    /// This class is necessary for initialization and cleaning of the windows wsa network subsystem. 
    /// It will be created before calling main and destroyed after exiting main. Works only on windows
    class WSA_Init_Cleanup
    {
    public:
        WSA_Init_Cleanup();

        ~WSA_Init_Cleanup();
    };

    extern WSA_Init_Cleanup WSA_IC;
    #endif

	/// Function for getting an ip address from a url.
	/// \param[in] url address to get ip
	/// \return Return host ip or empty string if cannot get host ip.
	std::string GetIpByURL(std::string url);

    /// Function to set maximal tcp message size
	void SetMaxTcpMessageSize(std::size_t maxTcpMessageSize);

    /// Function to get maximal tcp message size
	std::size_t GetMaxTcpMessageSize();

    /// Function to set maximal udp message size
	void SetMaxUdpMessageSize(std::size_t maxUdpMessageSize);
	
    /// Function to get maximal udp message size
	std::size_t GetMaxUdpMessageSize();

	/*!
		Wrapper over the send function. Allows you to send std::string. 
		\param[in] sock socket to send data 
		\param[in] message message to send
        
        \return Returns true in case of success, false if it was disconnection 

        Description of the protocol. Information about the length of the message is added before the transmitted message. 
        The information is provided in the following form: the length of the message is translated into 
        a number system with a base of 128, and then each digit of the resulting number is stored in a separate byte. 
        After that, for all bytes of the number, except the last one, the first bit is set to 1. 
        This allows you to read the incoming message one character at a time, 
        and determine when the information about the length of the message ends and start reading the data itself.
        Byte write order - little endian
	*/
	bool Default_TCP_Send(EN_SOCKET sock, const std::string& message);

	/*!
		Wrapper over the recv function. Allows you to recv std::string. 
		\param[in] sock socket to get data
		\param[out] message string to put received data
        
		\return Returns true in case of success, false if it was disconnection

        Description of the protocol. Information about the length of the message is added before the transmitted message. 
        The information is provided in the following form: the length of the message is translated into 
        a number system with a base of 128, and then each digit of the resulting number is stored in a separate byte. 
        After that, for all bytes of the number, except the last one, the first bit is set to 1. 
        This allows you to read the incoming message one character at a time, 
        and determine when the information about the length of the message ends and start reading the data itself.
        Byte write order - little endian 
	*/ 
	bool Default_TCP_Recv(EN_SOCKET sock, std::string& message);

	/*!
		Wrapper over the sendto function. Allows you to send std::string. 
		\param[in] sock socket to send data
		\param[in] ipAddress destination ip address and port
		\param[in] message string to put received data

		\return Returns true in case of success, false if it was disconnection 
	*/ 
	void Default_UDP_Send(EN_SOCKET sock, std::string destinationAddress, const std::string& message);

	/*!
		Wrapper over the recv function. Allows you to recv std::string. 
		\param[in] sock socket to get data
		\param[in] ipAddress source ip address and port
		\param[out] message string to put received data

		\return Returns true in case of success, false if it was disconnection 
	*/ 
	bool Default_UDP_Recv(EN_SOCKET sock, std::string& sourceAddress, std::string& message);

	/// Close socket
	void CloseSocket(EN_SOCKET sock);

	/*!
		Divides the string to std::vector<std::string>. 
		\param[in] stringToSplit string to split
		\param[in] splitterString the string that divides the string. Default set to space(" ")
		\return Vector of strings
	*/ 
	std::vector<std::string> Split(const std::string& stringToSplit, const std::string& splitterString = " ");

	/*!
		Find all substring occurrences and put theirs positions inside result vector
		\param[in] stringToFindIn string to split
		\param[in] splitterString the string that divides the string. Default set to space(" ")
		\return Vector of strings
	*/ 
    std::vector<std::size_t> FindAllOccurrences(const std::string& stringToFindIn, const std::string& splitterString);

	/*! 
		\brief The function gets socket and filename and send file to socket. 
		\param[in] fileSendSocket the socket for sending files
		\param[in] filePath the full path to the file to be sent
		\param[in] isStop the reference to a atomic boolean variable to stop file transfer.
        \warning It is important to understand that using this variable, you can stop the file sending, 
        but the receiving party will not know about it. In order for the receiving side to stop waiting for the file data, 
        it is necessary to stop receiving the file on the receiving side
        \param[in] microsecondsBetweenSendingChunks the parameter is needed to regulate the file transfer rate. Set time in microseconds between sending 4 kilobytes. 0 means no delay
		\param[in] previouslySendedSize the parameter is needed to continue downloading. Gets the size of the previously transmitted file in bytes. 0 means no previosly sending
        \param[in] fileTransmissionStatus A link to an object to track the file transfer status. After each iteration of the dispatch loop, all internal variables of the class are updated. I
        If desired, you can set a function in it that will be called once a second
		
		\return Returns true in case of file transmition success, false otherwise
		
		By default, the library progress function is used to output to the console.
	*/ 
	bool SendFile(EN_SOCKET fileSendSocket, std::string filePath, std::atomic_bool& isStop, std::atomic_int& microsecondsBetweenSendingChunks,
		uint64_t previouslySendedSize, EN_FileTransmissionStatus& fileTransmissionStatus);

	/*!
		\brief This function will wait incoming file.

        If a file with the same name already exists, the function will create a new one with a postscript (1). 
        Each subsequent file will have a postscript 1 more. At the beginning of receiving the file, 
        it will be saved with the postscript tmp. After successful receipt of the file, the tmp postscript will be removed
		\param[in] FileSendSocket the socket for receiving files
		\param[in] IsStop the reference to a atomic boolean variable to stop file transfer.
        \warning It is important to understand that using this variable, you can stop the file receiving, 
        but the sending party will not know about it. In order for the sending side to stop waiting for the file data, 
        it is necessary to stop sending the file on the sending side and you need to do this before calling the file reception stop
		\param[in] fileTransmissionStatus A link to an object to track the file transfer status. After each iteration of the dispatch loop, all internal variables of the class are updated. I
        If desired, you can set a function in it that will be called once a second
        
		\return Returns true in case of file transmition success, false otherwise
	*/
	bool RecvFile(EN_SOCKET FileSendSocket, std::atomic_bool& IsStop, EN_FileTransmissionStatus& fileTransmissionStatus);

	/*!
		\brief This function will forward file from one socket to another.
		\param[in] SourceFileSocket the source socket for receiving files
		\param[in] DestinationFileSocket the destination socket for sending files
		\param[in] IsStop the reference to a atomic boolean variable to stop file transfer. 
		\param[in] fileTransmissionStatus A link to an object to track the file transfer status. After each iteration of the dispatch loop, all internal variables of the class are updated. I
        If desired, you can set a function in it that will be called once a second

		\return Returns true in case of file transmition success, false otherwise
	*/
	bool ForwardFile(EN_SOCKET SourceFileSocket, EN_SOCKET DestinationFileSocket, std::atomic_bool& IsStop, EN_FileTransmissionStatus& fileTransmissionStatus);

	/*!
		\brief This function will print information about file downloading
		\param[in] thisSessionTransferedBytes the first is how many bytes were transmitted.
		\param[in] fileSize the second is the total number of bytes.
		\param[in] transmissionSpeed the third parameter shows the transfer rate in bytes.
		\param[in] transmissionEta the fourth shows the remaining time in seconds
	*/
	void DefaultDownloadStatusFunction(uint64_t thisSessionTransferedBytes, uint64_t fileSize, uint64_t transmissionSpeed, uint64_t transmissionEta);

	/// Return true if file exists, otherwise rerurn false
	bool IsFileExist(std::string filePath);

	/// \brief Return file size in bytes
	/// If couldn't open the file return 0
	uint64_t GetFileSize(std::string fileName);

	/// Returns the number of processor cores
	int GetCPUCores();

    
    /// Return last socket error code 
    int GetSocketErrorCode();

    /** 
        Return string description to socket error
        \param [in] socketErrorCode Socket error code. Optional parameter.
        If you do not specify it, the function itself will take the last socket error from the operating system
    */ 
    std::string GetSocketErrorString(int socketErrorCode = -1);
    
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

    /// Returns the name of today's day of the week
    std::string GetCurrentDayWeek();

    /** 
        \brief Function for getting the current date
        \param [in] IsAmericanFormat A variable for specifying the format of the returned date. By default, the parameter is false
        If the parameter is empty or false, the function returns the date in normal format: dd.mm.yyyy.
        Otherwise it will return in the american format: mm.dd.yyyy.
        \return Return current date in string with format: dd.mm.yyyy or mm.dd.yyyy depends on input param
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
        \brief Set operating system socket options
        Wrapper over setsockopt operating system function.
        \param [in] socket Socket to set options
        \param [in] level Socket option level
        \param [in] optionName Socket option name
        \param [in] optionValue Socket option value
    */
    void SetSocketOption(EN_SOCKET socket, int level, int optionName, int optionValue);

    /// Function to generate 4 version uuid
    std::string UUID4();

    /// A function for checking a string for any characters other than numbers
    bool IsCanBeDigit(const std::string& str);

    /**
        \brief A function for converting a string to a int.
        
        \param [in] str The string to be converted
        \param [in] res The number where the number from the string will be written, if it was possible to convert

        \return Returns true if the conversion succeeded and false if it failed
    */ 
    bool StringToInt(const std::string& str, int& res);

    /**
        \brief A function for converting a string to a long int.
        
        \param [in] str The string to be converted
        \param [in] res The number where the number from the string will be written, if it was possible to convert

        \return Returns true if the conversion succeeded and false if it failed
    */ 
    bool StringToInt(const std::string& str, long int& res);

    /**
        \brief A function for converting a string to a long long int.
        
        \param [in] str The string to be converted
        \param [in] res The number where the number from the string will be written, if it was possible to convert

        \return Returns true if the conversion succeeded and false if it failed
    */ 
    bool StringToInt(const std::string& str, long long int& res);

    /**
        \brief A function for converting a string to a unsigned long int.
        
        \param [in] str The string to be converted
        \param [in] res The number where the number from the string will be written, if it was possible to convert

        \return Returns true if the conversion succeeded and false if it failed
    */ 
    bool StringToInt(const std::string& str, unsigned long int& res);

    /**
        \brief A function for converting a string to a unsigned long long int.
        
        \param [in] str The string to be converted
        \param [in] res The number where the number from the string will be written, if it was possible to convert

        \return Returns true if the conversion succeeded and false if it failed
    */ 
    bool StringToInt(const std::string& str, unsigned long long int& res);

	/**
        \brief Crossplatform function for program suspension

        By default, the function takes milliseconds, but using the type definition, 
        you can change the type of the accepted value
        For example:
        EN::Delay(100); Will suspend thread for 100 milliseconds
        EN::Delay<std::chrono::microseconds>(100); Will suspend thread for 100 microseconds

        Possible specializations:
        std::chrono::nanoseconds
        std::chrono::microseconds
        std::chrono::milliseconds
        std::chrono::seconds
        std::chrono::minutes
        std::chrono::hours
    */
    template <class T = std::chrono::milliseconds>
	void Delay(uint64_t timeToWait)
    {
        std::this_thread::sleep_for(T(timeToWait));
    }

    /**
        \brief This function returns a fraction of a current second.

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
        const auto now = std::chrono::system_clock::now();
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
		\brief Function to convert int to string with base 256. 
		Works with standart data types. Use this function if your data 
		types support next operators: >>, <<, &

        \warning This function does not work with negative numbers.

		One char takes 1 byte, and a int takes 4 bytes. 
		If you try to translate the number 120 into a string, then you will have 3 characters or 3 bytes.
		This function turns a number into a character, since a character occupies a byte, 
		then you can transfer numbers up to 255 in one byte. 
		This function can be considered as the translation of a number into a number system with a base of 256
		This function does not work with negative numbers.
	*/
	template <class T>
    std::string IntToB256String(T n)
    {
        std::string str;

        if (n == 0) str += '\0';
        
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

        \warning This function does not work with negative numbers.

		One char takes 1 byte, and a int takes 4 bytes. 
		If you try to translate the number 120 into a string, then you will have 3 characters or 3 bytes.
		This function turns a number into a character, since a character occupies a byte, 
		then you can transfer numbers up to 255 in one byte. 
		This function can be considered as the translation of a number into a number system with a base of 256
	*/
	template <class T>
    std::string CIntToB256String(T n)
    {
        std::string str;

        if (n == 0) str += '\0';
        
        while(n > 0)
        {
            str += (unsigned char)(n % T(256));
            n /= 256;
        }

        return str;
    }

	/** \brief Function to convert base 256 string to int.
	
		Works with standart data types. Use this function if your data 
		types support next operators: >>, <<, &
		Works with strings from function EN::IntToB256String.
		\warning Dont forget to specify returning value using <type>. Example: int i = B256StringToInt<int>("string")
        \warning Don't forget that the IntToB256String function doesn't work with negative numbers
	*/
    template <class T>
    T B256StringToInt(const std::string& str)
    {
        T n = 0;
        for (auto it = str.rbegin(); it != --str.rend(); ++it)
        {
            n += (unsigned char)*it;
            n *= 256;
        }
        n += (unsigned char)str[0];
        return n;
    }

	/** \brief Function to convert base 256 string to int.
	
		Works with custom data types. Use this function if your data 
		types dont support next operators: >>, <<, &
		Works with strings from function EN::CIntToB256String.
		\warning Dont forget to specify returning value using <type>. Example: int i = CB256StringToInt<int>("string")
        \warning Don't forget that the CIntToB256String function doesn't work with negative numbers
	*/
    template <class T>
    T CB256StringToInt(const std::string& str)
    {
        T n = 0;
        for (auto it = str.rbegin(); it != --str.rend(); ++it)
        {
            n += (unsigned char)*it;
            n <<= 8;
        }
        n += (unsigned char)str[0];
        return n;
    }
}