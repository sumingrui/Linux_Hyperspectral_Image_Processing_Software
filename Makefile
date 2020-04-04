hsproc: main.o bus.o share.o
	g++ -o hsproc main.o bus.o share.o -lconfig++

main.o: main.cc bus.h
	g++ -c main.cc -std=c++11 

bus.o: bus.cc share.h
	g++ -c bus.cc

share.o: share.cc
	g++ -c share.cc -std=c++11

.PHONY : clean
clean :
	rm hsproc main.o bus.o share.o



g++ share.cc bus.cc main.cc db/DBtools.cc db/hyperspectralDB.cc -std=c++11 -lconfig++ -lmysqlclient -o hsproc


