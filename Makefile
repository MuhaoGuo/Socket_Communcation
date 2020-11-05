.PHONY:serverA serverB mainserver

all: serverA.cpp serverB.cpp mainserver.cpp client1.cpp client2.cpp
	g++ -o serverA serverA.cpp
	g++ -o serverB serverB.cpp
	g++ -o mainserver mainserver.cpp
	g++ -o client1 client1.cpp
	g++ -o client2 client2.cpp

mainserver:
	./mainserver
serverA:
	./serverA
serverB:
	./serverB
client1:
	./client1
client2:
	./client2


