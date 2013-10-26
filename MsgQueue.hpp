
#pragma once

#include "Message.h"

class MsgQueue
{
public:
  MsgQueue(unsigned long maxSize);
  void send(unsigned long id, Message* msg = NULL);
  Message* receive(unsigned long &id);
  ~MsgQueue();
private:
  //Container with the queue
  //Other relevant variables
};