\mainpage
# Easy_Network
Simple C++ network library
# Library documentation:  
https://mrognor.github.io/Easy_Network_Documentation/doc/html/index.html  

# Library usage 
Before you start, I advise you to download the code examples. For linux, you can save files to one folder, and for visual studio, you will need to add these files to different projects  
Examples available here: https://github.com/mrognor/Easy_Network/tree/master/Examples

## Visual Studio library usage guide
1. Download the latest release from github
2. Create a new project in visual studio
3. Click project -> properties

![VisualStudioProjectProperties](https://user-images.githubusercontent.com/47296449/165805905-60992708-92ed-4107-a858-e94e13581369.png)

4. Make sure that the configuration is selected to "All Configurations"

Open menu on C/C++ -> General -> Additional Include Directories
and add the path to the include library folder there

![VisualStudioProjectInclude](https://user-images.githubusercontent.com/47296449/165806651-ee06a01e-4438-4b5b-a2e9-bf17fd1a938d.png)

5. Open menu on Linker -> General -> Additional Library Directories
and add the path to the lib library folder there

![image](https://user-images.githubusercontent.com/47296449/165807739-68487caa-84ea-4caf-8630-63198174dea2.png)

6. Open menu on Linker -> Input -> Additional Dependencies and add "EasyNetwork.lib" here. Note that you must specify the correct file name for each build configuration. For example, to compile the debug x86 version, you must specify "EasyNetwork-x86d.lib", to release x86 - "EasyNetwork-x86.lib", to debug x64 - "EasyNetwork-x64d.lib", to release x64 - "EasyNetwork-x64.lib". Also add "ws2_32.lib"
<img width="590" alt="image" src="https://user-images.githubusercontent.com/47296449/198310488-4a83e847-6c09-4909-9ef2-8d812059046c.png">

7. The configuration of the project for working with the library is completed. Now you can use the code samples and try to make own programm  

## MinGW library usage guide
1. Download the latest release from github
2. You can extract library release archive to any folder
3. Now all ready to compile your project  
You can compile server using command:  
`g++ -pthread -std=c++11 -lws2_32 PathToServer/Server.cpp -IEasyNetworkDir/ -LEasyNetworkDir/bin -lEasyNetwork -o server`  
You can compile client using command:   
`g++ -pthread -std=c++11 -lws2_32 PathToClient/Client.cpp -IEasyNetworkDir/ -LEasyNetworkDir/bin -lEasyNetwork -o client`  
4. Run your programs using command:  
`server.exe` and `client.exe`

## Linux library usage
1. Download the latest release from github
2. You can extract library release archive to any folder
3. Now all ready to compile your project  
You can compile server using command:  
`g++ -pthread -std=c++11 PathToServer/Server.cpp -IEasyNetworkDir/ -LEasyNetworkDir/bin -lEasyNetwork -o server`
You can compile client using command:   
`g++ -pthread -std=c++11 PathToClient/Client.cpp -IEasyNetworkDir/ -LEasyNetworkDir/bin -lEasyNetwork -o client` 
4. Run your programs using command:  
`./server` and `./client`

# Important
I know that no one reads further than the readme, so I want to bring some important things here  
1. When working with linux, you may notice that std::cout does not
output characters to the console. Don't worry, 
your code works as it should, you can check it yourself with cerr.  
2. On linux, after disconnecting clients from the server, 
the port may be blocked for a small amount of time. 
Due to when trying to start the server again, 
errors will come out. We just have to wait a little  
3. The UDP server cannot send a message to the client if it has not received a 
message from this client before. This does not apply to unreliable sending of 
messages in rau classes
4. Note that the maximum size of a udp packet that can be guaranteed to be sent and received is 508 bytes. You can send more, but then the message may not go away  

# Free white ip
White ip allow you to use your programs outside your local network.  
You can use free utility named [ngrok](https://ngrok.com/). This program let you free white ip.  
You can launch ngrok using command `ngrok tcp 1111`. You can use any protocol and port you want.  
Note that when using this program, URLs will be provided to you, but ip is used to connect. To solve this problem, use the `GetIpByURL` function

# How to choose a class  
This is a small overview of the library classes to simplify class selection

## TCP classes
These classes support connection and have got only reliable message sending.  
### EN_TCP_Server  
You have to define 3 methods
1. void OnClientConnected(int ClientID)  
2. void ClientMessageHandler(std::string message, int ClientID)  
3. void OnClientDisconnect(int ClientID)  

The server ip address and port are set in the constructor.  
To start the server, you need to call the Run method  
An example of using the class can be found here: https://github.com/mrognor/Easy_Network/blob/master/Examples/TCP_Chat_Server.cpp  

### EN_TCP_Client  
You have to define 3 methods  
1. void OnConnect()  
2. void ServerMessageHandler(std::string message)  
3. void OnDisconnect()  

To connect to the server, you need to call the function Connect.  
Before shutting down the program, you need to disconnect from the server using the method Disconnect  
An example of using the class can be found here: https://github.com/mrognor/Easy_Network/blob/master/Examples/TCP_Chat_Client.cpp  

## UDP classes  
These classes do not support connection and have got only unreliable message sending.  
### EN_UDP_Server  
You have to define 2 methods
1. void ClientMessageHandler(std::string message, std::string ClientIpAddress, long long TimeSincePackageArrived)
2. bool InstantClientMessageHandler(std::string message, std::string ClientIpAddress, long long TimeWhenPackageArrived)  

The entire configuration of the server takes place in the constructor.  
To start the server, you must call the method Run. 
To stop the server, you need to call the method Shutdown.  
An example of using the class can be found here: https://github.com/mrognor/Easy_Network/blob/master/Examples/UDP_Chat_Server.cpp  

### EN_UDP_Client  
You have to define 1 method
1. void ServerMessageHandler(std::string message)  

To start the client's work, you must call the method Run.  
To complete the work, you must call the method Close.  
An example of using the class can be found here: https://github.com/mrognor/Easy_Network/blob/master/Examples/UDP_Chat_Client.cpp  

## RAU classes
This class supports connection. There is both reliable sending of messages and unreliable.  
The RAU differs from the TCP only in the ability to unreliable message send.  
### EN_RAU_Server
You have to define 3 methods
1. void OnClientConnected(int ClientID)  
2. void ClientMessageHandler(std::string message, int ClientID)  
3. void OnClientDisconnect(int ClientID)  

The entire configuration of the server takes place in the constructor  
To start the server, you need to call the Run method  
An example of using the class can be found here: https://github.com/mrognor/Easy_Network/blob/master/Examples/RAU_Chat_Server.cpp  

### EN_RAU_Client  
You have to define 3 methods  
1. void OnConnect()  
2. void ServerMessageHandler(std::string message)  
3. void OnDisconnect()  

To connect to the server, you need to call the function Connect.  
Before shutting down the program, you need to disconnect from the server using the method Disconnect  
An example of using the class can be found here: https://github.com/mrognor/Easy_Network/blob/master/Examples/RAU_Chat_Client.cpp  

# Functions
[Info about all library functions:](namespace_e_n.html#func-members)