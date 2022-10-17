CPPFlags = -pthread
FileExt = sh

ifeq ($(OS),Windows_NT)
	CPPFlags += -lws2_32
	FileExt = exe
endif

all: file_sender tcp_chat udp_chat rau_chat

rau_chat: lib
	g++ -std=c++11 Examples/RAU_Chat_Server.cpp bin/libEasyNetwork.a -o bin/RAU_Chat_Server.$(FileExt) $(CPPFlags)
	g++ -std=c++11 Examples/RAU_Chat_Client.cpp bin/libEasyNetwork.a -o bin/RAU_Chat_Client.$(FileExt) $(CPPFlags)

file_sender: lib 
	g++ -std=c++11 Examples/TCP_FileSender_Server.cpp bin/libEasyNetwork.a -o bin/TCP_FileSender_Server.$(FileExt) $(CPPFlags)
	g++ -std=c++11 Examples/TCP_FileSender_Client.cpp bin/libEasyNetwork.a -o bin/TCP_FileSender_Client.$(FileExt) $(CPPFlags)

tcp_chat: lib
	g++ -std=c++11 Examples/TCP_Chat_Server.cpp bin/libEasyNetwork.a -o bin/TCP_Chat_Server.$(FileExt) $(CPPFlags)
	g++ -std=c++11 Examples/TCP_Chat_Client.cpp bin/libEasyNetwork.a -o bin/TCP_Chat_Client.$(FileExt) $(CPPFlags)

udp_chat: lib
	g++ -std=c++11 Examples/UDP_Chat_Server.cpp bin/libEasyNetwork.a -o bin/UDP_Chat_Server.$(FileExt) $(CPPFlags)
	g++ -std=c++11 Examples/UDP_Chat_Client.cpp bin/libEasyNetwork.a -o bin/UDP_Chat_Client.$(FileExt) $(CPPFlags)

lib: bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_TCP_FileSender.o bin/EN_UDP_Client.o bin/EN_UDP_Server.o bin/EN_RAU_Server.o bin/EN_RAU_Client.o
	ar rc bin/libEasyNetwork.a bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_TCP_FileSender.o bin/EN_UDP_Client.o bin/EN_UDP_Server.o bin/EN_RAU_Server.o bin/EN_RAU_Client.o
	ranlib bin/libEasyNetwork.a

bin/EN_RAU_Client.o: EN_RAU_Client.cpp
	mkdir -p bin
	g++ -std=c++11 -c EN_RAU_Client.cpp -o bin/EN_RAU_Client.o 

bin/EN_RAU_Server.o: EN_RAU_Server.cpp
	mkdir -p bin
	g++ -std=c++11 -c EN_RAU_Server.cpp -o bin/EN_RAU_Server.o 

bin/EN_UDP_Client.o: EN_UDP_Client.cpp
	mkdir -p bin
	g++ -std=c++11 -c EN_UDP_Client.cpp -o bin/EN_UDP_Client.o 

bin/EN_UDP_Server.o: EN_UDP_Server.cpp
	mkdir -p bin
	g++ -std=c++11 -c EN_UDP_Server.cpp -o bin/EN_UDP_Server.o 

bin/EN_TCP_FileSender.o: EN_TCP_FileSender.cpp
	mkdir -p bin
	g++ -std=c++11 -c EN_TCP_FileSender.cpp -o bin/EN_TCP_FileSender.o 
bin/EN_TCP_Server.o: EN_TCP_Server.cpp
	mkdir -p bin
	g++ -std=c++11 -c EN_TCP_Server.cpp -o bin/EN_TCP_Server.o 

bin/EN_TCP_Client.o: EN_TCP_Client.cpp
	mkdir -p bin
	g++ -std=c++11 -c EN_TCP_Client.cpp -o bin/EN_TCP_Client.o 

bin/EN_Functions.o: EN_Functions.cpp
	mkdir -p bin
	g++ -std=c++11 -c EN_Functions.cpp -o bin/EN_Functions.o 

clean:
	rm -rf bin
