#include "EN_Functions.h"

namespace EN
{
	void Send(SOCKET& sock, std::string message, int MessageDelay)
	{
		int msg_size = message.length();

		send(sock, (char*)&msg_size, sizeof(int), NULL);
		send(sock, message.c_str(), message.length(), NULL);
		Sleep(MessageDelay);
	}

	int Recv(SOCKET& sock, std::string& message)
	{
		int msg_size;
		int ConnectionStatus = recv(sock, (char*)&msg_size, sizeof(int), NULL);

		if (ConnectionStatus <= 0)
		{
			message = "";
			closesocket(sock);
			return ConnectionStatus;
		}

		char* msg = new char[msg_size + 1];
		msg[msg_size] = '\0';

		ConnectionStatus = recv(sock, msg, msg_size, NULL);

		if (ConnectionStatus <= 0)
		{
			message = "";
			closesocket(sock);
			return ConnectionStatus;
		}

		message = msg;
		delete[] msg;
		return ConnectionStatus;
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
}