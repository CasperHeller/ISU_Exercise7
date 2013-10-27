#include "MsgQueue.h"
#include <pthread.h>

// Mutex og conditional
pthread_mutex_t mQMutex;
pthread_cond_t mQCond;

MsgQueue::MsgQueue(unsigned long maxSize)
{
  size_ = maxSize;
  
  int err;
  
  if ((err = pthread_mutex_init(&mQMutex, NULL)) != 0)
  {
    cout << "Initalizing mutex error: " << err << endl;
  }
  
  if ((err = pthread_cond_init(&mQCond, NULL)) != 0)
  {
    cout << "Initalizing conditional error: " << err << endl;
  } 
}

void MsgQueue::send(unsigned long id, Message* msg = NULL)
{
  pthread_mutex_lock(&mQMutex);
  
  //Venter på køen ikke er fuld
  while(itemQueue_.size() >= size_);
  {
    pthread_cond_wait(&mQCond, &mQMutex);
  }
  
  //Laver et objekt der kan sendes til køen
  Item newItem;
  newItem.msg_ = msg;
  newItem.id_ = id;
  
  //Sender det til køen
  itemQueue_.push(newItem);
  
  pthread_cond_signal(&mQCond);
  pthread_mutex_unlock(&mQMutex);
}

Message* MsgQueue::receive(unsigned long &id)
{
  pthread_mutex_lock(&mQMutex);
  
  //Venter til køen ikke er tom
  while(itemQueue_.empty())
  {
    pthread_cond_wait(&mQCond, &mQMutex);
  }
  
  //Læser den ældste i køen og fjerner den derefter
  Item newItem;
  newItem = itemQueue_.front();
  itemQueue_.pop();
  
  //Gemmer ID
  id = newItem.id_;
  
  pthread_cond_signal(&mQCond);
  pthread_mutex_unlock(&mQMutex);
  
  //Returnerer den udtaget message
  return newItem.msg_;
}

MsgQueue::~MsgQueue();
{
  delete &itemQueue_;
  
  int err;
  
  if ((err = pthread_mutex_destroy(&mQMutex)) != 0)
  {
    cout << "Destroying mutex error: " << err << endl;
  }
  
  if ((err = pthread_cond_destroy(&mQCond)) != 0)
  {
    cout << "Destroying conditional error: " << err << endl;
  }
}