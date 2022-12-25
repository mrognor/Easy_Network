LDFLAGS = -pthread
CXXFLAGS = -std=c++11
FileExt = sh

ifeq ($(OS), Windows_NT)
	LDFLAGS += -lws2_32
	FileExt = exe
endif

debug: CXXFLAGS += -g
debug: all

release: CXXFLAGS += -D NDEBUG -O3
release: all

all: bin/TCP_Chat_Server.$(FileExt) bin/TCP_Chat_Client.$(FileExt) bin/UDP_Chat_Server.$(FileExt) bin/UDP_Chat_Client.$(FileExt) bin/RAU_Chat_Server.$(FileExt) bin/RAU_Chat_Client.$(FileExt) bin/TCP_FileSender_Server.$(FileExt) bin/TCP_FileSender_Client.$(FileExt) bin/ParallelFor.$(FileExt)

# TCP_Chat
bin/TCP_Chat_Server.$(FileExt): bin/libEasyNetwork.a Examples/TCP_Chat_Server.cpp
	g++ $(CXXFLAGS) Examples/TCP_Chat_Server.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_Chat_Server.$(FileExt) $(LDFLAGS)
	
bin/TCP_Chat_Client.$(FileExt): bin/libEasyNetwork.a Examples/TCP_Chat_Client.cpp
	g++ $(CXXFLAGS) Examples/TCP_Chat_Client.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_Chat_Client.$(FileExt) $(LDFLAGS)

# UDP_Chat
bin/UDP_Chat_Server.$(FileExt): bin/libEasyNetwork.a Examples/UDP_Chat_Server.cpp
	g++ $(CXXFLAGS) Examples/UDP_Chat_Server.cpp -I. -Lbin -lEasyNetwork -o bin/UDP_Chat_Server.$(FileExt) $(LDFLAGS)
	
bin/UDP_Chat_Client.$(FileExt): bin/libEasyNetwork.a Examples/UDP_Chat_Client.cpp
	g++ $(CXXFLAGS) Examples/UDP_Chat_Client.cpp -I. -Lbin -lEasyNetwork -o bin/UDP_Chat_Client.$(FileExt) $(LDFLAGS)

# RAU_Chat
bin/RAU_Chat_Server.$(FileExt): bin/libEasyNetwork.a Examples/RAU_Chat_Server.cpp
	g++ $(CXXFLAGS) Examples/RAU_Chat_Server.cpp -I. -Lbin -lEasyNetwork -o bin/RAU_Chat_Server.$(FileExt) $(LDFLAGS)
	
bin/RAU_Chat_Client.$(FileExt): bin/libEasyNetwork.a Examples/RAU_Chat_Client.cpp
	g++ $(CXXFLAGS) Examples/RAU_Chat_Client.cpp -I. -Lbin -lEasyNetwork -o bin/RAU_Chat_Client.$(FileExt) $(LDFLAGS)

# TCP_FileSender
bin/TCP_FileSender_Server.$(FileExt): bin/libEasyNetwork.a Examples/TCP_FileSender_Server.cpp
	g++ $(CXXFLAGS) Examples/TCP_FileSender_Server.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_FileSender_Server.$(FileExt) $(LDFLAGS)

bin/TCP_FileSender_Client.$(FileExt): bin/libEasyNetwork.a Examples/TCP_FileSender_Client.cpp
	g++ $(CXXFLAGS) Examples/TCP_FileSender_Client.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_FileSender_Client.$(FileExt) $(LDFLAGS)

# ParallelFor
bin/ParallelFor.$(FileExt): bin/libEasyNetwork.a Examples/ParallelFor.cpp
	g++ $(CXXFLAGS) Examples/ParallelFor.cpp -I. -Lbin -lEasyNetwork -o bin/ParallelFor.$(FileExt) $(LDFLAGS)
	
# Library binary
bin/libEasyNetwork.a: bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_TCP_FileSender.o bin/EN_UDP_Client.o bin/EN_UDP_Server.o bin/EN_RAU_Server.o bin/EN_RAU_Client.o
	ar rc bin/libEasyNetwork.a bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_TCP_FileSender.o bin/EN_UDP_Client.o bin/EN_UDP_Server.o bin/EN_RAU_Server.o bin/EN_RAU_Client.o
	ranlib bin/libEasyNetwork.a

# Build all object files
bin/EN_RAU_Client.o: EN_RAU_Client.cpp
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_RAU_Client.cpp -o bin/EN_RAU_Client.o 

bin/EN_RAU_Server.o: EN_RAU_Server.cpp
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_RAU_Server.cpp -o bin/EN_RAU_Server.o 

bin/EN_UDP_Client.o: EN_UDP_Client.cpp
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_UDP_Client.cpp -o bin/EN_UDP_Client.o 

bin/EN_UDP_Server.o: EN_UDP_Server.cpp
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_UDP_Server.cpp -o bin/EN_UDP_Server.o 

bin/EN_TCP_FileSender.o: EN_TCP_FileSender.cpp
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_TCP_FileSender.cpp -o bin/EN_TCP_FileSender.o 
bin/EN_TCP_Server.o: EN_TCP_Server.cpp
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_TCP_Server.cpp -o bin/EN_TCP_Server.o 

bin/EN_TCP_Client.o: EN_TCP_Client.cpp
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_TCP_Client.cpp -o bin/EN_TCP_Client.o 

bin/EN_Functions.o: EN_Functions.cpp
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_Functions.cpp -o bin/EN_Functions.o 

# Clean all
clean:
	rm -rf bin
