LDFLAGS = -pthread
CXXFLAGS = -std=c++11 -Wall
FILEEXT =
TARGETS = bin/EN_Functions.o \
	bin/EN_TCP_Client.o \
	bin/EN_TCP_Server.o \
	bin/EN_UDP_Client.o \
	bin/EN_UDP_Server.o \
	bin/EN_Logger.o \
	bin/EN_ThreadGate.o \
	bin/EN_ThreadCrossWalk.o \
	bin/EN_SocketOptions.o \
	bin/EN_BackgroundTimer.o \
	bin/EN_FileTransmissionStatus.o \
	bin/EN_FT_Server.o \
	bin/EN_FT_Client.o \
	bin/EN_ThreadBarrier.o\
	bin/EN_EXPERIMENTAL_HTTP_Server.o # bin/EN_RAU_Server.o bin/EN_RAU_Client.o

# Check os
ifeq ($(OS), Windows_NT)
	LDFLAGS += -lws2_32
	FILEEXT = .exe
endif

# Check native atomic_int64 support
ifneq ($(shell test -e ./scripts/CheckAtomicInt64Support.test && echo -n yes), yes)
	CXXFLAGS += -D NATIVE_ATOMIC_INT64_NOT_SUPPORTED
    TARGETS += bin/EN_Atomic_Int64.o
   	$(warning Your system does not have native atomic_int64 support. A software implementation will be used)
endif

#use -D DISABLE_LOGGER to disable logger

debug: CXXFLAGS += -g
debug: all

release: CXXFLAGS += -D NDEBUG -O3
release: all

all: bin/TCP_Chat_Server$(FILEEXT) \
	bin/TCP_Chat_Client$(FILEEXT) \
	bin/UDP_Chat_Server$(FILEEXT) \
	bin/UDP_Chat_Client$(FILEEXT) \
	bin/TCP_BlockingFileTransmitter_Server$(FILEEXT) \
	bin/TCP_BlockingFileTransmitter_Client$(FILEEXT) \
	bin/ParallelFor$(FILEEXT) \
	bin/HTTP_Server$(FILEEXT) \
	bin/FT_Chat_Server$(FILEEXT) \
	bin/FT_Chat_Client$(FILEEXT) \
	bin/HTTP_Server$(FILEEXT)
# bin/RAU_Chat_Server$(FILEEXT) bin/RAU_Chat_Client$(FILEEXT)

# HTTP_Server
bin/HTTP_Server$(FILEEXT): bin/libEasyNetwork.a Examples/http/HTTP_Server.cpp
	g++ $(CXXFLAGS) Examples/http/HTTP_Server.cpp -I. -Lbin -lEasyNetwork -o bin/HTTP_Server$(FILEEXT) $(LDFLAGS)
	cp -n Examples/http/index.html bin
	cp -n Examples/http/page.html bin
	cp -n Examples/http/404.html bin
	cp -n Examples/http/Picture.png bin
	cp -n Examples/http/Picture.jpg bin
	cp -n Examples/http/Picture.svg bin

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

# TCP_BlockingFileTransmitter
bin/TCP_BlockingFileTransmitter_Server$(FILEEXT): bin/libEasyNetwork.a Examples/TCP_BlockingFileTransmitter_Server.cpp
	g++ $(CXXFLAGS) Examples/TCP_BlockingFileTransmitter_Server.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_BlockingFileTransmitter_Server$(FILEEXT) $(LDFLAGS)

bin/TCP_BlockingFileTransmitter_Client$(FILEEXT): bin/libEasyNetwork.a Examples/TCP_BlockingFileTransmitter_Client.cpp
	g++ $(CXXFLAGS) Examples/TCP_BlockingFileTransmitter_Client.cpp -I. -Lbin -lEasyNetwork -o bin/TCP_BlockingFileTransmitter_Client$(FILEEXT) $(LDFLAGS)

# FileTransmitter_Chat
bin/FT_Chat_Server$(FILEEXT): bin/libEasyNetwork.a Examples/FT_Chat_Server.cpp
	g++ $(CXXFLAGS) Examples/FT_Chat_Server.cpp -I. -Lbin -lEasyNetwork -o bin/FT_Chat_Server$(FILEEXT) $(LDFLAGS)

bin/FT_Chat_Client$(FILEEXT): bin/libEasyNetwork.a Examples/FT_Chat_Client.cpp
	g++ $(CXXFLAGS) Examples/FT_Chat_Client.cpp -I. -Lbin -lEasyNetwork -o bin/FT_Chat_Client$(FILEEXT) $(LDFLAGS)

# ParallelFor
bin/ParallelFor$(FILEEXT): bin/libEasyNetwork.a Examples/ParallelFor.cpp
	g++ $(CXXFLAGS) Examples/ParallelFor.cpp -I. -Lbin -lEasyNetwork -o bin/ParallelFor$(FILEEXT) $(LDFLAGS)
	
# Library binary
bin/libEasyNetwork.a: $(TARGETS)
	ar rc bin/libEasyNetwork.a $(TARGETS)
	ranlib bin/libEasyNetwork.a

# Build all object files
# bin/EN_RAU_Client.o: EN_RAU_Client.cpp EN_RAU_Client.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
#	mkdir -p bin
#	g++ $(CXXFLAGS) -c EN_RAU_Client.cpp -o bin/EN_RAU_Client.o

# bin/EN_RAU_Server.o: EN_RAU_Server.cpp EN_RAU_Server.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h
# 	mkdir -p bin
#	g++ $(CXXFLAGS) -c EN_RAU_Server.cpp -o bin/EN_RAU_Server.o

bin/EN_FT_Client.o: EN_FT_Client.cpp EN_FT_Client.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h EN_ThreadBarrier.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_FT_Client.cpp -o bin/EN_FT_Client.o

bin/EN_FT_Server.o: EN_FT_Server.cpp EN_FT_Server.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h EN_ThreadBarrier.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_FT_Server.cpp -o bin/EN_FT_Server.o

bin/EN_UDP_Client.o: EN_UDP_Client.cpp EN_UDP_Client.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h EN_ThreadBarrier.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_UDP_Client.cpp -o bin/EN_UDP_Client.o

bin/EN_UDP_Server.o: EN_UDP_Server.cpp EN_UDP_Server.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h EN_ThreadBarrier.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_UDP_Server.cpp -o bin/EN_UDP_Server.o

bin/EN_TCP_Server.o: EN_TCP_Server.cpp EN_TCP_Server.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h EN_ThreadBarrier.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_TCP_Server.cpp -o bin/EN_TCP_Server.o

bin/EN_TCP_Client.o: EN_TCP_Client.cpp EN_TCP_Client.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h EN_ThreadBarrier.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_TCP_Client.cpp -o bin/EN_TCP_Client.o

bin/EN_Functions.o: EN_Functions.cpp EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h EN_ThreadBarrier.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_Functions.cpp -o bin/EN_Functions.o

bin/EN_Logger.o: EN_Logger.cpp EN_Logger.h EN_Functions.h EN_SocketOptions.h EN_ThreadGate.h EN_ThreadCrossWalk.h EN_ThreadBarrier.h
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

bin/EN_Atomic_Int64.o: EN_Atomic_Int64.cpp EN_Atomic_Int64.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_Atomic_Int64.cpp -o bin/EN_Atomic_Int64.o

bin/EN_ThreadBarrier.o: EN_ThreadBarrier.cpp EN_ThreadBarrier.h
	mkdir -p bin
	g++ $(CXXFLAGS) -c EN_ThreadBarrier.cpp -o bin/EN_ThreadBarrier.o

bin/EN_EXPERIMENTAL_HTTP_Server.o: experimental/EN_HTTP_Server.h experimental/EN_HTTP_Server.cpp
	mkdir -p bin
	g++ $(CXXFLAGS) -c experimental/EN_HTTP_Server.cpp -o bin/EN_EXPERIMENTAL_HTTP_Server.o

# Clean all
clean:
	rm -rf bin
