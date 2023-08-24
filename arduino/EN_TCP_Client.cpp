#include "TCP_Client.h"

bool TCP_Client::ConnectToWIFI(String wifiName, String wifiPassword, int attemptsCount)
{
    WiFi.begin(wifiName.c_str(), wifiPassword.c_str());

    Serial.println("Connecting to Wifi:");
    for (int i = 0; i < attemptsCount; ++i)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("\nConnected to WiFi.");
            return true;
        }
        delay(500);
        Serial.print(".");
    }
    return false;
}

bool TCP_Client::ConnectToServer(String serverIP, String serverPort, int attemptsCount)
{
    Serial.println("Connecting to server:");
    for (int i = 0; i < attemptsCount; ++i)
    {
        Client.connect(serverIP.c_str(), serverPort.toInt());
        if (Client.connected())
        {
            Serial.println("\nConnected to server.");
            return true;
        }
        delay(500);
        Serial.print(".");
    }
    return false;
}

bool TCP_Client::SendToServer(const std::string &msg)
{
    // Sending data prefix with information about message length
    std::string messageLengthString;
    std::size_t messageLength = msg.length();

    if (messageLength == 0)
        messageLengthString += '\0';

    // Convert message length to 128 base system
    while (messageLength > 0)
    {
        // Get last seven bits. Equals to % 128
        messageLengthString += (unsigned char)(messageLength & 0b01111111);
        messageLength >>= 7;
    }

    messageLength = msg.length();

    // Alloc memory to sending message
    unsigned char *msgBuf = new unsigned char[messageLengthString.length() + messageLength];

    // Set first bit in length bytes except last length byte
    for (std::size_t i = 0; i < messageLengthString.length() - 1; ++i)
        msgBuf[i] = ((unsigned char)messageLengthString[i]) | 0b10000000;

    msgBuf[messageLengthString.length() - 1] = messageLengthString[messageLengthString.length() - 1];

    // Fill sending buffer with data from string
    std::size_t counter = messageLengthString.length();
    for (auto &it : msg)
    {
        msgBuf[counter] = (unsigned char)it;
        ++counter;
    }

    // Send all data in one send call
    int sendedBytes = Client.write((char *)msgBuf, messageLengthString.length() + messageLength);
    delete[] msgBuf;

    return ((size_t)sendedBytes == messageLengthString.length() + messageLength);
}

bool TCP_Client::WaitMessage(std::string &msg)
{
    std::string messageSizeString;
    int receivedBytes;

    // We read the incoming data one byte at a time until we get the first byte with the first bit equal to 0,
    // which means that this is the last byte with the size.
    while (true)
    {
        while (Client.available() < 1 && Client.connected())
        {
            volatile int i = 0;
        }

        uint8_t sizeByte;
        receivedBytes = Client.read(&sizeByte, 1);

        if (!Client.connected() || receivedBytes < 1)
            return false;

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

    // Allocate memory to incoming message
    char *msgBuf = new char[messageSize];

    // while (Client.available() < messageSize && Client.connected()) { volatile int i = 0; }

    // if (!Client.connected())
    //     return false;

    // receivedBytes = Client.read((uint8_t*)msgBuf, messageSize);

    // if ((std::size_t)receivedBytes != messageSize)
    // {
    // 	msg = "";
    // 	delete[] msgBuf;
    // 	return false;
    // }

    std::size_t counter = 0;
    while (counter < messageSize)
    {
        while (Client.available() < 1 && Client.connected())
        {
            volatile int i = 0;
        }

        if (!Client.connected())
            return false;

        msgBuf[counter] = Client.read();
        ++counter;
    }

    msg.clear();
    for (std::size_t i = 0; i < messageSize; ++i)
        msg += msgBuf[i];

    delete[] msgBuf;
    return true;
}

bool TCP_Client::GetMessage(std::string &msg)
{
    if (Client.available() != 0)
    {
        WaitMessage(msg);
        return true;
    }
    else
        return false;
}

void TCP_Client::DisconnectFromServer()
{
    Client.stop();
}

bool TCP_Client::IsConnectedToServer()
{
    return Client.connected();
}
