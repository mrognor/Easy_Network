#include "EN_TCP_Client.h"

namespace EN
{
	/// Client class for sending files to the server
	class EN_TCP_FileSender : public EN::EN_TCP_Client
	{
	private:

		/// A function to be defined by the user. It is used for logic after connection
		void AfterConnect() {};

		/// This method does nothing but we have to redefine it to work correctly
		void Run() {};

		/// This method does nothing but we have to redefine it to work correctly
		void ServerMessageHandler(std::string message) {};

		/// This method does nothing but we have to redefine it to work correctly
		void BeforeDisconnect() {};

		/// Variable to stop file transmission
		bool IsStop = false;

	public:
		EN_TCP_FileSender() : EN_TCP_Client() {};

		/**
			\brief Function to send file to server.

			\param[in] FileName name of file to be sent
			\param[in] Pointer to progress function. By default set to library function which displays progress in terminal
			\return Return true if file transmission success, false otherwise
		*/
		bool SendFileToServer(std::string FileName, void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta), int DelayInMilliseconds = 0);

		/// Function to recv file from server
		/// \return Return true if file transmission success, false otherwise
		bool RecvFileFromServer(void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta));

		/// Function to continue receiving file from server
		/// \return Return true if file transmission success, false otherwise
		bool ContinueRecvFileFromServer(void (*ProgressFunction)(uint64_t current, uint64_t all, uint64_t speed, uint64_t eta));

		/// Function to get message from server
		void RecvMessageFromServer(std::string& msg);

		/// Function to stop file transmission
		void StopDownloading() { IsStop = true; }
	};
}