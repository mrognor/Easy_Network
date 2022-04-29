
all:
	mkdir -p bin 
	g++ EN_TCP_Server.cpp EN_Functions.cpp Examples/TCPServer.cpp -pthread -o bin/TCPServer.sh 
	g++ EN_TCP_Client.cpp EN_Functions.cpp Examples/TCPClient.cpp -pthread -o bin/TCPClient.sh 

TCPServer: EN_TCP_Server.o EN_Functions.o TCPServer.o
	mkdir bin
	g++ EN_TCP_Server.o EN_Functions.o Examples/TCPServer.o -pthread -o TCPServer.sh 

TCPServer.o: Examples/TCPServer.cpp
	mkdir bin
	g++ -c Examples/TCPServer.cpp -o bin/TCPServer.o

EN_TCP_Server.o: EN_TCP_Server.cpp
	mkdir bin
	g++ -c EN_TCP_Server.cpp -o bin/EN_TCP_Server.o


Example_TCP_Client: EN_TCP_Client.o EN_Functions.o Client.o
	g++ EN_TCP_Client.o EN_Functions.o Client.o -pthread -o client.sh 

Client.o: Client.cpp
	g++ -c Client.cpp

EN_TCP_Client.o: EN_TCP_Client.cpp
	g++ -c EN_TCP_Client.cpp 


EN_Functions.o: EN_Functions.cpp
	g++ -c EN_Functions.cpp bin/EN_Functions.o

clean:
	rm -r *.o
	rm -r *.sh