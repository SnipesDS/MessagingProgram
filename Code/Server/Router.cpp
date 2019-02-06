//
//  Router.cpp
//  Server
//
//  Created by Christopher Palmer on 9/28/18.
//  Copyright © 2018 Personal. All rights reserved.
//

#include "Router.h"


/**
 *@brief Returns the one instance of Holder
 */
Router::Router(){}
#if 0
static Holder* getInstance(){
    static Holder *h=NULL;                              // Create an empty Holder m_instance
    if(m_intstnace ==NULL){                             // If m_instance is empty
        m_instnace =new Holder();                       // Make a new m_instance
    }                                                   //
    return m_instance;                                  // Return m_instance
}
#endif

/**
 *@brief Adds handle to the client list
 *@param h handle to add to client list
 */
void Router::addHandle(Handle *h){
    clients.push_back(h);
    cout<<"List of clients is now "<<clients.size()<<" long"
    ".\n"<<h->name<<" was just added to the list.\n";
    char msg[MESSAGELENGTH];
    strcpy(msg,h->name);
    strcat(msg," has connected");
    deliverMessage(NULL, msg, "Server");
}

/**
 *@brief Removes handle with the given name from list
 *@return Returns true if client was removed
 */
bool Router::removeHanlde(const char *n){
    for(vector<Handle *>::iterator itr = clients.begin(); itr != clients.end(); ++itr){                  // Search all clients
        if((*itr)->name==n){                // If handle's name matches the give name
            char msg[MESSAGELENGTH];
            strcpy(msg,(*itr)->name);
            strcat(msg," has disconnected");
            clients.erase(itr);                       // Remove them from the list
            //cout<<n<<" was removed from the list.\n";
            deliverMessage(NULL, msg, "Server");
            return true;                                // Return true
        }
    }                                                   // If name was not found
    return false;                                       // Return false
}

/**
 *@brief Compares names to know if given name is unique
 *@return True if name is unique and false if not
 */
bool Router::unique(const char* name){
    for(vector<Handle *>::iterator itr = clients.begin(); itr != clients.end(); ++itr){                 // For all the current Handles
        if(strcmp((*itr)->name,name)==0){   // Compare their name to the given name
            return false;                               // If they match, return false
        }
    }                                                   // Since none of the handle's names
    return true;                                        //   match, this name is unique
}

/**
 *@brief Creates and delivers message to rep from sen
 *@param rep is the recipient of the message
 *@param msg is the full text of the message
 *@param sen is the message of the sender
 *@return Returns true if message was delivered
 and false if the rep was not found
 */
bool Router::deliverMessage(const char *rep, const char *msg, const char *sen){
    Message m;                                          // Create message to deliver
    strcpy(m.sender,sen);                               // Copy sen to message's sender
    strcpy(m.text,msg);                                 // Copy msg to message's text (full message)
    if(rep==NULL){
        cout<<"rep is NULL\n";
    }
    if(rep != NULL && strlen(rep) != 0){                                      // If there is a recipient
        for(vector<Handle *>::iterator itr = clients.begin(); itr != clients.end(); ++itr){              // Search all handles
            if(strcmp((*itr)->name,rep)==0){          // If handle's name matches recipient
                (*itr)->inbox.push(m);           // Deliver message to handle
                cout<<(*itr)->name<<" just recieved a Message.\n";
                return true;                            // Return that the message was delivered
            }
        }
    }else{                                              // If there was no recipient
        for(vector<Handle *>::iterator itr = clients.begin(); itr != clients.end(); ++itr){              // For every current handle
            (*itr)->inbox.push(m);               // Deliver the message
            cout<<(*itr)->name<<" just recieved a Message.\n";
        }
        return true;                                    // Return that the message was delivered
    }
    return false;                                       // Return that recipient was not found
}

/**
 *@brief Deconstructor for Holder
 */
Router::~Router(){
    for(vector<Handle *>::iterator itr = clients.begin(); itr != clients.end(); ++itr){                         // For every client
        cout<<"Deleting "<<(*itr)->name<<" from Router.\n";
        clients.erase(itr);
    }
}
