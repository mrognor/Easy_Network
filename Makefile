all: file_sender chat

file_sender: lib 
	g++ Examples/TCP_FileSender_Server.cpp bin/libEasyNetwork.a -pthread -o bin/TCP_FileSender_Server.sh 
	g++ Examples/TCP_FileSender_Client.cpp bin/libEasyNetwork.a -pthread -o bin/TCP_FileSender_Client.sh

chat: lib
	g++ Examples/TCP_Chat_Server.cpp bin/libEasyNetwork.a -pthread -o bin/TCP_Chat_Server.sh 
	g++ Examples/TCP_Chat_Client.cpp bin/libEasyNetwork.a -pthread -o bin/TCP_Chat_Client.sh 

lib: bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_TCP_FileSender.o
	ar rc bin/libEasyNetwork.a bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_TCP_FileSender.o
	ranlib bin/libEasyNetwork.a

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
