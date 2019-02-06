//
//  main.cpp
//  Server
//
// Christopher Palmer
//
// Not all of this code is my code. The initial set up of the server
//    is code I got from http://beej.us/guide/bgnet/html/multi/clientserver.html
//    It is a great guide for learning the basics of network programming
//    in a unix enviornment. There is a chat room code in the next chapter
//    but I wanted to figure out some of the problems myself.
// The sections of beej.us code is marked in between the following:
////////////////////////////////////////////////////////////////////////
//////////////////////////    beej.us code    //////////////////////////
    // code here
    // code here
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////

// Everything else is my own code and will be commented.

////////////////////////////////////////////////////////////////////////
//////////////////////////    beej.us code    //////////////////////////
/*
 ** server.c -- a stream socket server demo
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////

#include <ctype.h>
#include <pthread.h>
#include <sys/time.h>
#include "Connection.h"

#define PORT "47000"        // the port users will be connecting to

////////////////////////////////////////////////////////////////////////
//////////////////////////    beej.us code    //////////////////////////
#define BACKLOG 10          // how many pending connections queue will hold
#define MAXDATASIZE 512     // max number of bytes we can send/recieve at once
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////
//////////////////////////    beej.us code    //////////////////////////
// This signal handler tries to catch zombie threads
void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////

// Descriptions are found below main witht the functions
int recieve(int sockfd, char *buf, long &numbytes);
int parse(char *buf, char *rep, char *msg);
void *client(void *h);

////////////////////////////////////////////////////////////////////////
//////////////////////////    beej.us code    //////////////////////////
int main(void){
    int sockfd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    //socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    //char s[INET6_ADDRSTRLEN];
    int rv;
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////
    
    bool running=true;              // Used to know if the server is running
    vector<pthread_t *> connections;// Container for any client threads spawned (one per client)
    Router *router= new Router();   // Used pass messages between client threads
                                    //   Every thread gets this one router to deliver messages

////////////////////////////////////////////////////////////////////////
//////////////////////////    beej.us code    //////////////////////////
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen)==-1){
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(servinfo); // all done with this structure
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    printf("server: waiting for connections...\n");
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////
    // While the server is running
    while(running){
        // Structure for the thread
        Connection *connection= new Connection(sockfd,their_addr, router);
        pthread_t *ti=new pthread_t;    // Empty thread object
        // If able to start a new thread
        if(pthread_create(ti, NULL, client, reinterpret_cast<void *>(connection))==0){
            connections.push_back(ti);  // Add thread to container
        }
    }
    // Collecting all the client threads
    for(vector<pthread_t *>::iterator itr = connections.begin(); itr != connections.end(); ++itr){
        pthread_join(*(*itr),NULL);
    }
    delete router;                      // Delete the new router used to compunicate between threads
    return 0;
}

/**
 *@brief Kinda sorta filters message
 *@return Return -1 if there was an error
 Returns 0 if socket is closed
 Returns 2 if message is ok
 */
int recieve(int sockfd, char *buf, long &numbytes){
    // recv returns the number of bytes recieved
    // If the number of bytes recieved is -1, there was an error
    if (( numbytes = recv(sockfd, buf , MAXDATASIZE-1, 0)) == -1){
        perror("recv");             // Print error message
        return -1;                  // Return -1
    // If the number of bytes recieved is 0, the socket is closed
    //   Return 0
    }else if(numbytes==0){return 0;}
    // If there was no error and the socket did not close
    //   This is a message to be passed on, return 2
    else{
        buf[numbytes] = '\0';
        return 2;
    }
}

/**
 *@brief parses the given buf into rep and msg
 *@return Returns 0 if Message is going to all
 Returns 1 if Message is not the right format
 Returns 2 if Message had recipient and text
 */
int parse(char *buf,char *rep, char *msg){
    char buff[MAXDATASIZE];
    bzero(buff, MAXDATASIZE);
    strncpy(buff, buf, MAXDATASIZE);
    bzero(rep,NAMELENGTH);
    bzero(msg,MESSAGELENGTH);
    //
    int pos = string(buff).find(":");   // Get the position of the first ':'
    //
    if (pos == string::npos) return 1;
    int msgLength=strlen(buff)-pos-1;
    //cout<<pos<<endl;
    if(pos==0) {
        rep=NULL;
        strncpy(msg,&buff[1],msgLength);
        return 0;
    }else if (pos != string::npos){
        strncpy(rep,buff, pos);
        strncpy(msg,&buff[pos+1],msgLength);
        //
        return 2;                       //
    }else{
        return 1;                       //
    }
}

/**
 *@brief Thread function
 *@param cont all the information needed by thread
 */
void *client(void *cont){
    long numbytes;                      // Used for storing the number of bytes received and to send
    char buf[MAXDATASIZE];              // Char array used receiving and sending messages
    bzero(buf,MAXDATASIZE);             // Clearing buf for use
    bool connected=true;                // Used to monitor the loop for sending and receiving
    Connection *connection=(Connection *)cont;  // Converting data structure back to readable format
    //////////////////*FINISHING* CONNECTION SET-UP//////////////////
    // Checking if connected
    if (connection->getSockfd() == -1) {
        // Connection failed
        perror("accept");               // Print error message
        connected=false;                // Set connection to false
        return NULL;                    // Exit routine
    }
    // Print out connection's IP address (can be IPv4 or IPv6)
    printf("server: got connection from %s\n", connection->getIP());
    // Asking for 'handle' or their user name
    //    This will be used by the Router to deliver messages to other clients
    if (send(connection->getSockfd(), "What's your handle?", 19, 0) == -1)
        perror("send");                 // If not able to send, print error message
    bool u;                             // Temp bool for knowing if handle is avaliable
    // This loop is to ensure that there are no duplicate names being entered into the Router
    do{
        // If not able to receive message
        if (int i=recieve(connection->getSockfd(), buf, numbytes)==-1){
            // Print error message
            perror("recv: handle");
        // If connection has been dropped
        }else if(i==0&&numbytes<=1){
            // Break out of loop
            connected=false;            // Set connetion to be false
            break;                      // Break out of verifcation loop
        }
        // Checking if handle is unique and storing result in u
        u=connection->getRouter()->unique(buf);
        // If handle is not unique
        if(!u){
            // Send 2 back to client (signals name was not accepted)
            send(connection->getSockfd(), "2",1,0);
        }
        // If name is unique
        else{
            // Send 1 back to client (signals name was accepted)
            send(connection->getSockfd(), "1",1,0);
            buf[numbytes]='\0';         // Add NULL terminator to end of name
            // Store name for later use
            strcpy(connection->getHandle()->name,buf);
            // Add handle structure to Router
            connection->getRouter()->addHandle(connection->getHandle());
            // Print to screen new connection to server and its handle
            printf("server: connection %s's handle is \"%s\"\n",connection->getIP(),connection->getHandle()->name);
            bzero(buf, numbytes);       // Clear buf for reuse
        }
    }while(!u);// If name is not unique, aks for a new handle. Otherwise, continue
    struct timeval window;              // Set up timeval for select()
    window.tv_sec=0;
    window.tv_usec=100000;              // Setting useconds to 0.1 sec
    fd_set read,write;                  // FileDescriptor sets used for select()
    char recipient[NAMELENGTH];         // Setting up char array to hold a recipient's name
    char message[MESSAGELENGTH];        // Setting up char array to hold a message from the client
    ///////////////////END OF CONNECTION SET-UP///////////////////
    // Loop of operations while connected
    while(connected){//////////////////// CONNECTED LOOP //////////////////////
// //////////////////*******START MESSAGE RECIEVED********////////////////// //
        FD_ZERO(&write);                // Ready write set for use
        // Adding socket information to write set
        FD_SET(connection->getSockfd(),&write);
        FD_SET(0,&write);               // Add Standard input to the write set
        FD_ZERO(&read);                 // Ready read set for use
        // Adding socket information to read set
        FD_SET(connection->getSockfd(),&read);
        // See which the provided FileDescriptors are open
        select(connection->getSockfd()+1, &read, &write, NULL, &window);
        // If socket is ready to read
        if(FD_ISSET(connection->getSockfd(),&read)){
            //If socket has not been closed
            if(int i=recieve(connection->getSockfd(), buf, numbytes)!=0){
                // Message has been received
                // If message is empty, do not post
                if(i==-1 || strlen(buf) == 0 || isspace((int)buf[0])) {
                    // Clear buf for reuse (this will break it out of read for now)
                    bzero(buf, numbytes);
                // If Message is not empty
                }else{
                    // Add NULL terminator
                    buf[numbytes]='\0';
                    // If Message is in the right format
                    if (parse(buf, recipient, message)!=1){
                        // Print Message to the screen and who it is from
                        printf("%s: %s\n",connection->getHandle()->name, buf);
                        // Clear buf for reuse
                        bzero(buf, numbytes+1);
                        // If Message was not delivered
                        if(!connection->getRouter()->deliverMessage(recipient, message, connection->getHandle()->name)){
                            // Create error message
                            char error[MAXDATASIZE];
                            // Add recipient to the begining of message
                            strcpy(error, recipient);
                            // Append error to end of error
                            strcat(error, " is not a valid recipient.");
                            // Send error message to cleint
                            send(connection->getSockfd(), error, MAXDATASIZE, 0);
                        }
                    // If Message was not the right format
                    }else{
                        // Send error message to cleint from server
                        send(connection->getSockfd(), "Server: Message not right format", 32, 0);
                    }
                }
            // If socket has been closed
            }else{
                connected=false;        // Set connected flag to false
                continue;               // Stop loop
            }
        }
// //////////////////********END MESSAGE RECIEVED*********////////////////// //
        FD_ZERO(&write);                // Clearing write for use
        // Adding socket information to write set
        FD_SET(connection->getSockfd(),&write);
        FD_ZERO(&read);                 // Clearing read for use
        // Adding socket information to read set
        FD_SET(connection->getSockfd(),&read);
        // See which the provided FileDescriptors are open
        select(connection->getSockfd(), &read, &write, NULL, &window);
        // If socket is ready to write and not read
        if(FD_ISSET(connection->getSockfd(),&write)||!FD_ISSET(connection->getSockfd(),&read)){
            // Send client Message from Inbox
            connection->checkInbox();
        }
    }//////////////////////////// EXITING CONNECTED LOOP /////////////////////////
    // Remove handle from Router's list
    while(!connection->getRouter()->removeHanlde(connection->getHandle()->name)){}
    // Print to screen connection has been closed
    printf("Connection with: \"%s\" has been closed.\n", connection->getHandle()->name);
    close(connection->getSockfd());     // Close socket before exiting
    return NULL;                        // Exit routine
}
