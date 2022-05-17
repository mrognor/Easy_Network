all: file_sender tcp_chat udp_chat

file_sender: lib 
	g++ Examples/TCP_FileSender_Server.cpp bin/libEasyNetwork.a -pthread -o bin/TCP_FileSender_Server.sh 
	g++ Examples/TCP_FileSender_Client.cpp bin/libEasyNetwork.a -pthread -o bin/TCP_FileSender_Client.sh

tcp_chat: lib
	g++ Examples/TCP_Chat_Server.cpp bin/libEasyNetwork.a -pthread -o bin/TCP_Chat_Server.sh 
	g++ Examples/TCP_Chat_Client.cpp bin/libEasyNetwork.a -pthread -o bin/TCP_Chat_Client.sh 

udp_chat: lib
	g++ Examples/UDP_Chat_Server.cpp bin/libEasyNetwork.a -pthread -o bin/UDP_Chat_Server.sh 
	g++ Examples/UDP_Chat_Client.cpp bin/libEasyNetwork.a -pthread -o bin/UDP_Chat_Client.sh 

lib: bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_TCP_FileSender.o bin/EN_UDP_Client.o bin/EN_UDP_Server.o
	ar rc bin/libEasyNetwork.a bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_TCP_FileSender.o bin/EN_UDP_Client.o bin/EN_UDP_Server.o
	ranlib bin/libEasyNetwork.a

bin/EN_UDP_Client.o: EN_UDP_Client.cpp
	mkdir -p bin
	g++ -c EN_UDP_Client.cpp -o bin/EN_UDP_Client.o

bin/EN_UDP_Server.o: EN_UDP_Server.cpp
	mkdir -p bin
	g++ -c EN_UDP_Server.cpp -o bin/EN_UDP_Server.o

bin/EN_TCP_FileSender.o: EN_TCP_FileSender.cpp
	mkdir -p bin
	g++ -c EN_TCP_FileSender.cpp -o bin/EN_TCP_FileSender.o

bin/EN_TCP_Server.o: EN_TCP_Server.cpp
	mkdir -p bin
	g++ -c EN_TCP_Server.cpp -o bin/EN_TCP_Server.o

bin/EN_TCP_Client.o: EN_TCP_Client.cpp
	mkdir -p bin
	g++ -c EN_TCP_Client.cpp -o bin/EN_TCP_Client.o

bin/EN_Functions.o: EN_Functions.cpp
	mkdir -p bin
	g++ -c EN_Functions.cpp -o bin/EN_Functions.o

clean:
	rm -rf bin
