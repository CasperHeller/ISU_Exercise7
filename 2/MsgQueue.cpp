#include "MsgQueue.h"
#include <pthread.h>

MsgQueue::MsgQueue(unsigned long maxSize)
{
    size_ = maxSize;
    int err;

    if ((err = pthread_mutex_init(&mQMutex, NULL)) != 0)
    {
        cout << "Error initalizing mutex: " << err << endl;
    }
    if ((err = pthread_cond_init(&mQCond, NULL)) != 0)
    {
        cout << "Error initalizing conditional: " << err << endl;
    }
}

void MsgQueue::send(unsigned long id, Message* msg)
{
    pthread_mutex_lock(&mQMutex);

    //Venter indtil køen ikke er fuld
    while (itemQueue_.size() >= size_)
    {
        pthread_cond_wait(&mQCond, &mQMutex);
    }

    //Laver et objekt af type item
    Item newItem;
    newItem.msg_ = msg;
    newItem.id_ = id;
    
    //Sender objektet
    itemQueue_.push(newItem);

    pthread_cond_signal(&mQCond);
    pthread_mutex_unlock(&mQMutex);
}

Message* MsgQueue::receive(unsigned long& id)
{
    pthread_mutex_lock(&mQMutex);

    //Venter indtil køen ikke er tom
    while (itemQueue_.empty())
    {
        pthread_cond_wait(&mQCond, &mQMutex);
    }

    //Læs det ældste objekt der er i køen og popper det fra køen
    Item newItem = itemQueue_.front();
    itemQueue_.pop();
    //Sender ID'et "by reference"
    id = newItem.id_;

    pthread_cond_signal(&mQCond);
    pthread_mutex_unlock(&mQMutex);

    //Returnere beskeden der blev poppet
    return newItem.msg_;
}

MsgQueue::~MsgQueue()
{
    int err;
    
    if ((err = pthread_mutex_destroy(&mQMutex)) != 0)
    {
        cout << "Error destroying mutex: " << err << endl;
    }
    if ((err = pthread_cond_destroy(&mQCond)) != 0)
    {
        cout << "Error destroying conditional: " << err << endl;
    }
}
