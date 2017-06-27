//
// Created by jesse_wang on 6/23/17.
// jessejcw@gmail.com
//

#include "SimpleServer.h"

int main() {

    SimpleServer my_server;
    if (my_server.start() != 0) {
        exit(0);
    }

    return 0;
}