#include "EN_TCP_Client.h"

class EN_TCP_FileSender : public EN::EN_TCP_Client
{
public:
	bool IsConnected = false;

	EN_TCP_FileSender();

	// A function to be defined by the user. It is used for logic after connection
	void AfterConnect();

	// This method does nothing but we have to redefine it to work correctly
	void Run() {};

	// Function to send file to server. First parametr is file name. Second is pointer to progress function
	void SendFileToServer(std::string FileName, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta));

	// This method does nothing but we have to redefine it to work correctly
	void ServerMessageHandler(std::string message) {};

	// This method does nothing but we have to redefine it to work correctly
	void BeforeDisconnect() {};
};