//
//  Router.h
//  Server
//
//  Created by Christopher Palmer on 9/28/18.
//  Copyright © 2018 Personal. All rights reserved.
//

#ifndef Router_h
#define Router_h

#include <stdio.h>
#include <vector>
#include <iostream>
#include "Handle.h"

class Router {
private:
    vector<Handle *> clients;
public:
    Router();
    ~Router();
    void addHandle(Handle *h);
    bool removeHanlde(const char *n);
    bool unique(const char* name);
    bool deliverMessage(const char *rep, const char *msg, const char *sen);
};
#endif /* Router_h */
