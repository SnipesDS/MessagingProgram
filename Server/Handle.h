//
//  Handle.h
//  Server
//
//  Created by Christopher Palmer on 9/28/18.
//  Copyright © 2018 Personal. All rights reserved.
//

#ifndef Handle_h
#define Handle_h

#include <stdio.h>

#include <queue>

#define MAXDATASIZE 512     // max number of bytes we can get at once
#define NAMELENGTH 20     // max length for a connection's handle
//                          // message may have more than one handle
#define MESSAGELENGTH 471   // max length for message text

using namespace std;

struct Message{
    char sender[NAMELENGTH];
    char text[NAMELENGTH+MESSAGELENGTH];
};

struct Handle{
    char name[NAMELENGTH];
    queue<Message> inbox;
    //mutex (Here for a more advanced version)
};

#endif /* Handle_h */
