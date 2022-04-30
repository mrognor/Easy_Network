
all:
	mkdir -p bin 
	g++ EN_TCP_Server.cpp EN_Functions.cpp Examples/TCPServer.cpp -pthread -o bin/TCPServer.sh 
	g++ EN_TCP_Client.cpp EN_Functions.cpp Examples/TCPClient.cpp -pthread -o bin/TCPClient.sh 

#TCPServer: EN_TCP_Server.o EN_Functions.o TCPServer.o
#	mkdir bin
#	g++ EN_TCP_Server.o EN_Functions.o Examples/TCPServer.o -pthread -o TCPServer.sh 

#TCPServer.o: Examples/TCPServer.cpp
#	mkdir -p bin
#	g++ -c Examples/TCPServer.cpp -o bin/TCPServer.o

#Example_TCP_Client: EN_TCP_Client.o EN_Functions.o Client.o
#	g++ EN_TCP_Client.o EN_Functions.o Client.o -pthread -o client.sh 

lib: EN_Functions.o EN_TCP_Client.o EN_TCP_Server.o
	mkdir -p bin
	ar rc bin/libEasyNetwork.a bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o
	ranlib bin/libEasyNetwork.a
	
EN_TCP_Server.o: EN_TCP_Server.cpp
	mkdir -p bin
	g++ -c EN_TCP_Server.cpp -o bin/EN_TCP_Server.o

EN_TCP_Client.o: EN_TCP_Client.cpp
	mkdir -p bin
	g++ -c EN_TCP_Client.cpp -o bin/EN_TCP_Client.o

EN_Functions.o: EN_Functions.cpp
	mkdir -p bin
	g++ -c EN_Functions.cpp -o bin/EN_Functions.o

clean:
	rm -rf bin
