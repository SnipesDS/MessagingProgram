//
//  Connection.cpp
//  Server
//
//  Created by Christopher Palmer on 9/28/18.
//  Copyright © 2018 Personal. All rights reserved.
//

#include "Connection.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family==AF_INET){
        return&(((struct sockaddr_in*)sa)->sin_addr);
    }
    return&(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 *@brief Handle constructor. creates Handle's socket file descriptor
 *@param sockfd the socket file descriptor where the connection was heard
 *@param their the connector's info
 *@param r a pointer to the Router
 */
Connection::Connection(int sockfd, struct sockaddr_storage their,Router* r){
    their_addr=their;
    sin_size=sizeof(their);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
    router=r;
    handle=new Handle;
}
/**
 *@brief Deletes the connection's handle
 */
Connection::~Connection(){
    delete handle;
}
/**
 *@brief Returns the connection's socket file descriptor
 *@return the connection's socket file descriptor
 */
const int Connection::getSockfd(){return new_fd;}
/**
 *@brief Returns the handle's human readable IP address
 *@return const char* of the handle's IP address
 */
const char* Connection::getIP(){return s;}
/**
 *@brief Returns a pointer to the Router
 *@return A pointer to the Router
 */
Router* Connection::getRouter(){return router;}
/**
 *@brief Sends Oldest Message in Inbox
 */
void Connection::checkInbox(){
    if(!handle->inbox.empty()){
        Message m;
        char buf[MAXDATASIZE];
        bzero(buf,MAXDATASIZE);
        m=handle->inbox.front();
        strcpy(buf, m.sender);
        buf[strlen(buf)]=':';
        strcat(buf, m.text);
        //cout<<buf<<endl;
        fd_set write;
        struct timeval window;                          //window cannot be a pointer
        window.tv_sec=0;
        window.tv_usec=100000;
        bool sent=false;
        while(!sent){
            FD_ZERO(&write);
            FD_SET(new_fd,&write);
            select(new_fd+1,NULL,&write,NULL,&window);
            if(FD_ISSET(new_fd,&write)){
                if(send(new_fd, buf, MAXDATASIZE, 0)!=-1){
                    handle->inbox.pop();
                    //cout<<"Message sent to "<<handle->name<<endl;
                    sent=true;
                }
            }
        }
    }
    return;
}
/**
 *@brief Retunrs a pointer to the connection's handle
 *@return A pointer to the connection's handle
 */
Handle* Connection::getHandle(){
    return handle;
}
