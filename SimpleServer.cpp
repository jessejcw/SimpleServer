//
// Created by jesse_wang on 6/23/17.
// jessejcw@gmail.com
//

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <zconf.h>
#include <cassert>
#include <cstring>
#include "SimpleServer.h"

#define ERROR_NONE  0
#define ERROR_SOME -1

#define MAX_CONNECTION 10

SimpleServer::~SimpleServer() {

}

int SimpleServer::start() {

    if (createWorkers() != ERROR_NONE) {
        return ERROR_SOME;
    }
    int listen_socket = -1;
    if (startServer(&listen_socket) != ERROR_NONE) {
        return ERROR_SOME;
    }
    if (handleConnect(listen_socket) != ERROR_NONE) {
        return ERROR_SOME;
    }
    return ERROR_NONE;
}

int SimpleServer::createWorkers() {
    int err_code = workr.createWorkers();
    return err_code;
}

int SimpleServer::startServer(int* listen_socket) {

    // socket connection
    struct sockaddr_in server_addr;
    int lsocket = socket(AF_INET, SOCK_STREAM, 0);
    if (lsocket < 0 ) {
        std::cout << "\n[ERROR] socket establish failed..." <<std::endl;
        return ERROR_SOME;
    }

    // Fill the addr struct
    memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(get_port_num());

    std::string ip_str = inet_ntoa(server_addr.sin_addr);
    std::cout << "\n[SUCCESS] server on socket:" << lsocket
         << " ip:"<<ip_str.c_str()<< " port:"<< ntohs(server_addr.sin_port) << std::endl;

    // bind the socket
    int error = bind(lsocket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (error == -1) {
        std::cout << "\n[ERROR] bind() failed at "<< __FILE__ << ":"<< __LINE__<< std::endl;
        return ERROR_SOME;
    }

    // listen
    listen(lsocket, MAX_CONNECTION);
    std::cout << "server listening.." << std::endl;

    *listen_socket = lsocket;
    return ERROR_NONE;
}



int processJob(void*, int);

int SimpleServer::handleConnect(int listen_socket) {

    assert(listen_socket>0);
    if (listen_socket < 0) { return ERROR_SOME; }

    fd_set master, read_fds;
    int fdMax;
    struct sockaddr_in client_;
    socklen_t conn = sizeof(client_);
    int client_socket[MAX_CONNECTION];
    memset(client_socket, 0, sizeof(client_socket));


    while (true) {

        FD_ZERO(&read_fds);

        FD_SET(listen_socket, &read_fds);
        fdMax = listen_socket;

        // add child sockets to set
        for (int i =0; i < MAX_CONNECTION; ++i) {
            int socket_decriptr = client_socket[i];

            if (socket_decriptr > 0) {
                FD_SET(socket_decriptr, &read_fds);
            }

            if (socket_decriptr > fdMax)
                fdMax = socket_decriptr;
        }

        int new_fd = select(fdMax+1, &read_fds, NULL, NULL, NULL);

        if (new_fd < 0) {
            perror("[ERROR]: select");
        }

        if (FD_ISSET(listen_socket, &read_fds)) {

            int new_socket = 0;
            if ((new_socket = accept(listen_socket, (struct sockaddr*)&client_, &conn)) < 0) {
                perror("[ERROR]: accept");
            }
            for (int i =0; i < MAX_CONNECTION; ++i) {
                if (client_socket[i] == 0) {
                    client_socket[i] = new_socket;

                    //packaging job
                    JobPkg job = {(void*)this, processJob, client_socket[i]};
                    workr.enqueueJob(job);
                    break;
                }
            }
        }
    }
}

int processJob(void* pData, int fd_num) {

    std::cout<< "processing..." << std::endl;
    SimpleServer* simpleServer = static_cast<SimpleServer*>(pData);

    int read_size;
    char buffer[BUFSIZ];

    while( (read_size = (int)recv(fd_num, buffer, BUFSIZ, 0))> 0) {
        buffer[read_size] = '\0';

        std::cout<< " recv msg:" << buffer << std::endl;

        write(fd_num, buffer, read_size);
    }
    return 0;
}
