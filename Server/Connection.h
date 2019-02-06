//
//  Connection.h
//  Server
//
//  Created by Christopher Palmer on 9/28/18.
//  Copyright © 2018 Personal. All rights reserved.
//

#ifndef Connection_h
#define Connection_h

#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
//#include <netinet/in.h>
//#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h> // for select
#include "Router.h"


class Connection{
    socklen_t sin_size;                                     //
    struct sockaddr_storage their_addr;                     //
    char s[INET6_ADDRSTRLEN];                               // Will hold a human readbale version of the connection's IP address
    int new_fd;                                             // Handle's socket file descriptor
    Handle *handle;
    Router *router;
public:
    Connection(int sockfd, struct sockaddr_storage their,Router* r);
    ~Connection();
    const int getSockfd();
    const char* getIP();
    Router* getRouter();
    void checkInbox();
    Handle* getHandle();
private:
    //void sendMessages();
};

#endif /* Connection_h */
