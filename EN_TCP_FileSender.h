#include "EN_TCP_Client.h"

class EN_TCP_FileSender : public EN::EN_TCP_Client
{
private:

	// A function to be defined by the user. It is used for logic after connection
	void AfterConnect() {};

	// This method does nothing but we have to redefine it to work correctly
	void Run() {};

	// This method does nothing but we have to redefine it to work correctly
	void ServerMessageHandler(std::string message) {};

	// This method does nothing but we have to redefine it to work correctly
	void BeforeDisconnect() {};

	bool IsStop = false;

public:
	EN_TCP_FileSender() : EN_TCP_Client() {};

	// Function to send file to server. First parametr is file name. Second is pointer to progress function
	bool SendFileToServer(std::string FileName, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta));

	// Function to recv file from server
	bool RecvFileFromServer();

	// Function to get message from server
	bool RecvMessageFromServer(std::string& msg);

	void StopDownloading() { IsStop = true; }
};