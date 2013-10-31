#include "msgqueue.h"
#include <pthread.h>

MsgQueue::MsgQueue(unsigned long maxSize)
{
    size_ = maxSize;
    int errno;

    // Initializing mutex and conditinal
    if ((errno = pthread_mutex_init(&msgMutex, NULL)) != 0)
    {
        cout << "Error initalizing mutex: " << errno << endl;
    }
    if ((errno = pthread_cond_init(&msgCond, NULL)) != 0)
    {
        cout << "Error initalizing conditional: " << errno << endl;
    }
}

void MsgQueue::send(unsigned long id, Message* msg)
{
    pthread_mutex_lock(&msgMutex);

    // Wait untill the queue is not full
    while (itemQueue_.size() >= size_)
    {
        pthread_cond_wait(&msgCond, &msgMutex);
    }

    // Generate an item to push
    Item item;
    item.msg_ = msg;
    item.id_ = id;
    // Now push it
    itemQueue_.push(item);

    pthread_cond_signal(&msgCond);
    pthread_mutex_unlock(&msgMutex);
}

Message* MsgQueue::receive(unsigned long& id)
{
    //pthread_mutex_lock(&msgMutex);

    // Wait untill the queue is not empty
    while (itemQueue_.empty())
    {
        pthread_cond_wait(&msgCond, &msgMutex);
    }

    // Read the oldest entry, and then pop it off the queue
    Item item = itemQueue_.front();
    itemQueue_.pop();
    // Pass the id by reference
    id = item.id_;

    pthread_cond_signal(&msgCond);
    pthread_mutex_unlock(&msgMutex);

    // Finally return message that was popped
    return item.msg_;
}

MsgQueue::~MsgQueue()
{
    int errno;
    // Destroying mutex and conditinal
    if ((errno = pthread_mutex_destroy(&msgMutex)) != 0)
    {
        cout << "Error destroying mutex: " << errno << endl;
    }
    if ((errno = pthread_cond_destroy(&msgCond)) != 0)
    {
        cout << "Error destroying conditional: " << errno << endl;
    }
}

MsgQueue::MsgQueue(const MsgQueue& other)
{
    size_ = other.size_;
    int errno;

    // Initializing mutex and conditinal
    if ((errno = pthread_mutex_init(&msgMutex, NULL)) != 0)
    {
        cout << "Error initalizing mutex: " << errno << endl;
    }
    if ((errno = pthread_cond_init(&msgCond, NULL)) != 0)
    {
        cout << "Error initalizing conditional: " << errno << endl;
    }
}
