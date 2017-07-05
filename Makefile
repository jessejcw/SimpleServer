all:
	g++ -g --std=gnu++11 SimpleServer.cpp Worker.h main.cpp -o run -lpthread
clean:
	rm run
