#pragma once
#include <iostream> // for NULL pointer definition
#include <queue>
#include "message.h"

using namespace std;



class MsgQueue
{
public:
    MsgQueue(unsigned long maxSize);
    void send(unsigned long id, Message* msg = NULL);
    MsgQueue(const MsgQueue& other);	// copy constructor
    Message* receive(unsigned long& id);
    ~MsgQueue();
private:
    // Definition of Item
    struct Item
    {
        unsigned long id_;
        Message* msg_;
    };

    queue<Item> itemQueue_;
    unsigned long size_;
    pthread_mutex_t msgMutex;
    pthread_cond_t msgCond;
};

