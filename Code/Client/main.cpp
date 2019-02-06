//
//  main.cpp
//  Client
//
// Christopher Palmer
//
// Not all of this code is my code. The initial set up of the client
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
 ** client.c -- a stream socket client demo
 */

#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////

#include <iostream>

#define PORT "47000"                // Port client will be connecting to
                                    //    where the server is listening

////////////////////////////////////////////////////////////////////////
//////////////////////////    beej.us code    //////////////////////////
#define MAXDATASIZE 512     // max number of bytes we can get at once
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////

#define HANDLELENGTH 20             // max length of connection's handle
#define MESSAGELENGTH 471           // max length of Message to send
#define SENDPROMPT "<<: "           // Input Prompt
#define RECVPROMPT ">> "            // Output Prompt

using namespace std;

////////////////////////////////////////////////////////////////////////
//////////////////////////    beej.us code    //////////////////////////
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family==AF_INET){return &(((struct sockaddr_in*)sa)->sin_addr);}
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////
    
    long numRecv,numSend;
    char bufRecv[MAXDATASIZE];
    char bufSend[MAXDATASIZE];
    bool state=true;                    // Used to stop program
    bool go=false;                      // The bool for checking if a message needs to be sent
    bzero(bufRecv,MAXDATASIZE);
    bzero(bufSend,MAXDATASIZE);

////////////////////////////////////////////////////////////////////////
//////////////////////////    beej.us code    //////////////////////////
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure
    if ((numRecv = recv(sockfd, bufRecv, MAXDATASIZE-1, 0)) == -1) {
        perror("recv");
        exit(1);
    }
    bufRecv[numRecv] = '\0';
    printf("client: received \n%s\n",bufRecv);
////////////////////////// ^^ //////////// ^^ //////////////////////////
////////////////////////// ^^ beej.us code ^^ //////////////////////////
////////////////////////////////////////////////////////////////////////
    
    bzero(bufRecv, numRecv+1);
    scanf("%s",bufSend);
    numSend=strlen(bufSend);
    bool test=false;
    do{
        while(numSend>HANDLELENGTH&&numSend>=0){
            bzero(bufSend,numSend);
            cout<<"Handle cannot be longer than 20 characters.\n"
            "Your handle was "<<numSend<<" long. Please try again.\n"
            "What's your handle?\n";
            cin>>bufSend;
            numSend=strlen(bufSend);
            cout<<numSend<<endl;
        }
        if(send(sockfd, bufSend, numSend, 0)==-1){
            perror("client: handle");
        }
        recv(sockfd, bufRecv, MAXDATASIZE-1, 0);
        if(bufRecv[0]=='2'){
            cout<<"Handle is already taken. Please choose another.\n";
            test=true;
            bzero(bufSend,numSend);
            numSend=HANDLELENGTH+1;
        }else if(bufRecv[0]=='1'){test=false;}
        //cout<<"Finished test\n";
    }while(test);
    bzero(bufSend, numSend);
////////////////INSTRUCTIONS///////////////////////
    printf("  Server instructions:\n");
    printf("When sending Messages, please write in the form:\n"
           "\tReciever:Message\n"
           "If sending to all, leave Reciever blank.\n Only one Reciever at a time.\n");
    printf("Enter 'exit' to disconnect\n" SENDPROMPT "0");
    struct timeval window;              //
    window.tv_sec=0;
    window.tv_usec=10000;
    int msgRecv=0,msgSent=0;
    fd_set read,write;                  // Read and write cannot be pointers
    
/////////////While Client is running///////////////
    while(state){
        FD_ZERO(&write);
        FD_SET(sockfd,&write);
        FD_ZERO(&read);
        FD_SET(sockfd,&read);
        FD_SET(0,&read);
        select(sockfd+1, &read, &write, NULL, &window);
        //////////Sending Message//////////////////
        if(FD_ISSET(0,&read)&&!FD_ISSET(sockfd,&read)){
            // If not trying to send a message
            if(!go){
                // Read in a new one
                cin.getline(bufSend, MESSAGELENGTH);
            }
            // If bufSend = 'exit'
            if (!strncmp(bufSend, "exit", 4)){
                // Close connection
                state=false;
            // If message is empty
            }else if(strncmp(bufSend, "\0", 1)==0){
                // Ignore message for sending
                //    (fall through if statements)
            // If message is not empty and not 'exit'
            }else{
                // Check to see if we can send a message
                if(!FD_ISSET(sockfd,&read)){
                    // If not able to send the message
                    if(send(sockfd, bufSend, MAXDATASIZE, 0)<1){
                        // Print error message
                        perror("client: sending Message");
                        //Mark the go flag to try again
                        go=true;
                    // If message was sent
                    }else{
                        // Clear bufSend
                        bzero(bufSend, MAXDATASIZE);
                        // Mark go flag to get next message
                        go=false;
                        // Increment sent message count (not nessecary)
                        msgSent++;
                        // Print new prompt
                        cout<<SENDPROMPT;
                    }
                // If not able to send message, Mark go flag to send message next time
                }else{go=true;}
            }
        }
        //////////End Sending Message//////////////
        FD_ZERO(&write);
        FD_SET(sockfd,&write);
        select(sockfd+1, NULL, &write, NULL, &window);
        //
        if(FD_ISSET(sockfd,&read)){
            //
            if ((numRecv = recv(sockfd, bufRecv, MAXDATASIZE-1, 0)) == -1){
                perror("recv");
                exit(1);
            }else if(numRecv==0){
                state=false;
                cout<<RECVPROMPT<<"Recieved: "<<bufRecv<<" with numRecv: "<<numRecv<<endl;
                cout<<"Server was shut down.\n";
            }else if(numRecv==-1 || strlen(bufRecv) == 0 || isspace((int)bufRecv[0])) { //If message is empty, don't post
                bzero(bufRecv, numRecv);
            }else{
                //
                cout<<"\n" RECVPROMPT<<bufRecv<<endl;
                cout<<SENDPROMPT;
                cout.flush();
                bzero(bufRecv, numRecv+1);
                msgRecv++;
            }
        }
    }
//////////////////// WHILE CLIENT IS RUNNING ///////////////////////
    close(sockfd);
    cout<<"Recieved "<<msgRecv<<" messages\n";
    cout<<"Sent "<<msgSent<<" messages\n";
    return 0;
}
