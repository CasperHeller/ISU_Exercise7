#pragma once

#include <iostream>
#include <queue>
#include "Message.h"

using namespace std;

class MsgQueue
{
public:
  MsgQueue(unsigned long maxSize);
  void send(unsigned long id, Message* msg = NULL);
  Message* receive(unsigned long& id);
  ~MsgQueue();
private:
  struct Item
  {
    unsigned long id_;
    Message* msg_;
  };
  
  queue<Item> itemQueue_;
  unsigned long size_;
  pthread_mutex_t mQMutex;
  pthread_cond_t mQCond;
};