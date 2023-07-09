LDFLAGS = -pthread
CXXFLAGS = -std=c++11 -Wall
FILEEXT =

ifeq ($(OS), Windows_NT)
	LDFLAGS += -lws2_32
	FILEEXT = .exe
endif

#use -D DISABLE_LOGGER to disable logger

debug: CXXFLAGS += -g
debug: all

release: CXXFLAGS += -D NDEBUG -O3
release: all

all: bin/TCP_Chat_Server$(FILEEXT) bin/TCP_Chat_Client$(FILEEXT) bin/UDP_Chat_Server$(FILEEXT) bin/UDP_Chat_Client$(FILEEXT) bin/TCP_FileSender_Server$(FILEEXT) bin/TCP_FileSender_Client$(FILEEXT) bin/ParallelFor$(FILEEXT)
# bin/RAU_Chat_Server$(FILEEXT) bin/RAU_Chat_Client$(FILEEXT)

# TCP_Chat
bin/TCP_Chat_Server$(FILEEXT): bin/libEasyNetwork.a Examples/TCP_Chat_Server.cpp
	g++ $(CXXFLAGS) Examples/TCP_Chat_Server.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_Chat_Server$(FILEEXT) $(LDFLAGS)
	
bin/TCP_Chat_Client$(FILEEXT): bin/libEasyNetwork.a Examples/TCP_Chat_Client.cpp
	g++ $(CXXFLAGS) Examples/TCP_Chat_Client.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_Chat_Client$(FILEEXT) $(LDFLAGS)

# UDP_Chat
bin/UDP_Chat_Server$(FILEEXT): bin/libEasyNetwork.a Examples/UDP_Chat_Server.cpp
	g++ $(CXXFLAGS) Examples/UDP_Chat_Server.cpp -I. -Lbin -lEasyNetwork -o bin/UDP_Chat_Server$(FILEEXT) $(LDFLAGS)
	
bin/UDP_Chat_Client$(FILEEXT): bin/libEasyNetwork.a Examples/UDP_Chat_Client.cpp
	g++ $(CXXFLAGS) Examples/UDP_Chat_Client.cpp -I. -Lbin -lEasyNetwork -o bin/UDP_Chat_Client$(FILEEXT) $(LDFLAGS)

# RAU_Chat
# bin/RAU_Chat_Server$(FILEEXT): bin/libEasyNetwork.a Examples/RAU_Chat_Server.cpp
#	g++ $(CXXFLAGS) Examples/RAU_Chat_Server.cpp -I. -Lbin -lEasyNetwork -o bin/RAU_Chat_Server$(FILEEXT) $(LDFLAGS)
	
# bin/RAU_Chat_Client$(FILEEXT): bin/libEasyNetwork.a Examples/RAU_Chat_Client.cpp
#	g++ $(CXXFLAGS) Examples/RAU_Chat_Client.cpp -I. -Lbin -lEasyNetwork -o bin/RAU_Chat_Client$(FILEEXT) $(LDFLAGS)

# TCP_FileSender
bin/TCP_FileSender_Server_Blocking$(FILEEXT): bin/libEasyNetwork.a Examples/TCP_FileSender_Server_Blocking.cpp
	g++ $(CXXFLAGS) Examples/TCP_FileSender_Server_Blocking.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_FileSender_Server_Blocking$(FILEEXT) $(LDFLAGS)

bin/TCP_FileSender_Client_Blocking$(FILEEXT): bin/libEasyNetwork.a Examples/TCP_FileSender_Client_Blocking.cpp
	g++ $(CXXFLAGS) Examples/TCP_FileSender_Client_Blocking.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_FileSender_Client_Blocking$(FILEEXT) $(LDFLAGS)

# ParallelFor
bin/ParallelFor$(FILEEXT): bin/libEasyNetwork.a Examples/ParallelFor.cpp
	g++ $(CXXFLAGS) Examples/ParallelFor.cpp -I. -Lbin -lEasyNetwork -o bin/ParallelFor$(FILEEXT) $(LDFLAGS)
	
# Library binary
bin/libEasyNetwork.a: bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_UDP_Client.o bin/EN_UDP_Server.o bin/EN_Logger.o bin/EN_ThreadGate.o bin/EN_ThreadCrossWalk.o bin/EN_SocketOptions.o bin/EN_BackgroundTimer.o bin/EN_FileTransmissionStatus.o # bin/EN_RAU_Server.o bin/EN_RAU_Client.o
	ar rc bin/libEasyNetwork.a bin/EN_Functions.o bin/EN_TCP_Client.o bin/EN_TCP_Server.o bin/EN_UDP_Client.o bin/EN_UDP_Server.o bin/EN_Logger.o bin/EN_ThreadGate.o bin/EN_ThreadCrossWalk.o bin/EN_SocketOptions.o bin/EN_BackgroundTimer.o bin/EN_FileTransmissionStatus.o
	ranlib bin/libEasyNetwork.a

# Build all object files
# bin/EN_RAU_Client.o: EN_RAU_Client.cpp EN_RAU_Client.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
#	mkdir -p bin
#	g++ $(CXXFLAGS) -c EN_RAU_Client.cpp -o bin/EN_RAU_Client.o

# bin/EN_RAU_Server.o: EN_RAU_Server.cpp EN_RAU_Server.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
# 	mkdir -p bin
#	g++ $(CXXFLAGS) -c EN_RAU_Server.cpp -o bin/EN_RAU_Server.o

bin/EN_UDP_Client.o: EN_UDP_Client.cpp EN_UDP_Client.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_UDP_Client.cpp -o bin/EN_UDP_Client.o

bin/EN_UDP_Server.o: EN_UDP_Server.cpp EN_UDP_Server.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_UDP_Server.cpp -o bin/EN_UDP_Server.o

bin/EN_TCP_Server.o: EN_TCP_Server.cpp EN_TCP_Server.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_TCP_Server.cpp -o bin/EN_TCP_Server.o

bin/EN_TCP_Client.o: EN_TCP_Client.cpp EN_TCP_Client.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_TCP_Client.cpp -o bin/EN_TCP_Client.o

bin/EN_Functions.o: EN_Functions.cpp EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_Functions.cpp -o bin/EN_Functions.o

bin/EN_Logger.o: EN_Logger.cpp EN_Logger.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_Logger.cpp -o bin/EN_Logger.o

bin/EN_SocketOptions.o: EN_SocketOptions.h EN_SocketOptions.cpp EN_Functions.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_SocketOptions.cpp -o bin/EN_SocketOptions.o

bin/EN_ThreadGate.o: EN_ThreadGate.cpp EN_ThreadGate.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_ThreadGate.cpp -o bin/EN_ThreadGate.o

bin/EN_ThreadCrossWalk.o: EN_ThreadCrossWalk.cpp EN_ThreadCrossWalk.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_ThreadCrossWalk.cpp -o bin/EN_ThreadCrossWalk.o

bin/EN_BackgroundTimer.o: EN_BackgroundTimer.cpp EN_BackgroundTimer.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_BackgroundTimer.cpp -o bin/EN_BackgroundTimer.o

bin/EN_FileTransmissionStatus.o: EN_FileTransmissionStatus.cpp EN_FileTransmissionStatus.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_FileTransmissionStatus.cpp -o bin/EN_FileTransmissionStatus.o

# Clean all
clean:
	rm -rf bin
