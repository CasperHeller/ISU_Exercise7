#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "MsgQueue.h"
#include "Message.h"

using namespace std;

//Struct
struct Point3D : public Message
{
  int x;
  int y;
  int z;
};

//Enum
enum
{
  ID_Point3D = 0
};

//Konstanter
const int MAX_QUEUE = 12;
const int SLEEP_TIME = 2;
const int MAX_X = 7;
const int MAX_Y = 14;
const int MAX_Z = 21;

//Prototyper
void *sender(void* data);
void *receiver(void* data);
void handler(Message* msg, unsigned long id);
void printPoint3D(Point3D* point);

int main()
{
  MsgQueue myQueue(MAX_QUEUE);
  pthread_t senderThread, receiverThread;
  int err;
  
  //senderThread
  if( (err = pthread_create(&senderThread, NULL, sender, static_cast<void*> (&myQueue))) != 0 )
  {
    cout << "Could not create senderThread! ERROR: " << err << endl;
    return EXIT_FAILURE;
  }
  
  //receiverThread
  if( (err = pthread_create(&receiverThread, NULL, receiver, static_cast<void*> (&myQueue))) != 0 )
  {
    cout << "Could not create receiverThread! ERROR: " << err << endl;
    return EXIT_FAILURE;
  }
  
  //Join
  if ( (err = pthread_join(senderThread, NULL)) )
  {
    cout << "Could not join senderThread! ERROR: " << err << endl;
    return EXIT_FAILURE;
  }
  
  if ( (err = pthread_join(receiverThread, NULL)) )
  {
    cout << "Could not join senderThread! ERROR: " << err << endl;
    return EXIT_FAILURE;
  }
  
  cout << "Main function is now done!" << endl;
  return EXIT_SUCCESS;
}

void *sender(void* data)
{
  MsgQueue *myQueue = static_cast<MsgQueue*> (data);
  
  for(;;)
  {
    Point3D *P3D = new Point3D;
    P3D->x = (rand()%MAX_X+1);
    P3D->y = (rand()%MAX_Y+1);
    P3D->z = (rand()%MAX_Z+1);
    
    myQueue->send(ID_Point3D, static_cast<Message*> (P3D));
    
    sleep(SLEEP_TIME);
  }
  
  pthread_exit(NULL);
}

void *receiver(void* data)
{
  MsgQueue *myQueue = static_cast<MsgQueue*> (data);
  
  for(;;)
  {
    unsigned long id;
    Message *msg = myQueue->receive(id);
    handler(msg, id);
    delete msg;
  }
  
  pthread_exit(NULL);
}

void handler(Message* msg, unsigned long id)
{
  switch(id)
  {
    case ID_Point3D:
    {
      Point3D* P3D = static_cast<Point3D*> (msg);
      printPoint3D(P3D);
      break;
    }
   
    default:
      cout << "The ID: " << id << " do not exist!" << endl;
      break;
    }
}

void printPoint3D(Point3D* point)
{
  cout << "The coordinates: (" << point->x << ", " << point->y << ", " << point->z << ")" << endl;
}