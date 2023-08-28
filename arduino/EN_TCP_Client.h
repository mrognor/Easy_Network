#ifndef TCP_Client_H
#define TCP_Client_H

#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif

#include <string>
#include "lwip/sockets.h"

class TCP_Client
{
protected:
    WiFiClient Client;
public:
    bool ConnectToWIFI(String  wifiName, String  wifiPassword, int attemptsCount = 20);

    bool ConnectToServer(String serverIP, String  serverPort, int attemptsCount = 20);
    bool SendToServer(const std::string&  msg);

    // Blocking call. Wait new message and put it in msg.
    // Return false if server was disonnected, otherwise return true
    bool WaitMessage(std::string& msg);

    // Non-blocking call. If have new message put it in msg
    // Return true if it was new message and put it inside msg, otherwise return true
    bool GetMessage(std::string& msg);

    void DisconnectFromServer();
    bool IsConnectedToServer();
};

#endif