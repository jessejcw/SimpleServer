all:
	g++ -g SimpleServer.cpp Worker.h main.cpp -o run -lpthread
clean:
	rm run
