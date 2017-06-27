//
// Created by jesse_wang on 6/23/17.
// jessejcw@gmail.com
//

#ifndef SIMPLESERVER_SIMPLESERVER_H
#define SIMPLESERVER_SIMPLESERVER_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Worker.h"

#define SRVR_DEFAULT_PORT 9800

class SimpleServer {
    //int listen_socket;
    int server;
    int port_num = SRVR_DEFAULT_PORT;
    bool is_exist = false;
    char buffer[BUFSIZ];
    struct sockaddr_in server_addr;
    socklen_t size;
    Worker workr;

public:
    ~SimpleServer();
    int get_port_num() const {
        return port_num;
    }

    void set_port_num(int port_num) {
        SimpleServer::port_num = port_num;
    }

    int start();
    int startServer(int*);
    int createWorkers();
    int handleConnect(int);
};


#endif //SIMPLESERVER_SIMPLESERVER_H
