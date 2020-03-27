CXX=g++
CXXFLAGS= -Wall

all: receiver.o sender.o

sender.o: sender.cpp msg.h
	$(CXX) $(CXXFLAGS) sender.cpp -o sender.o

receiver.o: receiver.cpp msg.h
	$(CXX) $(CXXFLAGS) receiver.cpp -o receiver.o

clean:
	rm -rf *o