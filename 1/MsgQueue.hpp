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
  Message* receive(unsigned long &id);
  ~MsgQueue();
private:
  queue<item> itemQueue_;
  unsigned long size_;
  
  struct Item
  {
    unsigned long id_;
    Message* msg_;
  };
};