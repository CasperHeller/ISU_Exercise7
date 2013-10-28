#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include "MsgQueue.h"
#include "Point3D.h"

using namespace std;

//Enum
enum
{
  ID_Point3D = 0
};

//Konstanter
const int MAX_QUEUE = 7;
const int SLEEP_TIME = 1;
const int MAX_X = 25;
const int MAX_Y = 50;
const int MAX_Z = 75;

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
  MsgQueue* myQueue = static_cast<MsgQueue*>(data);
  
  //Seed til generering af tilfældige tal
  srand(time(NULL));
  
  for(;;)
  {
    Point3D* P3D = new Point3D;
    P3D->x = rand() % MAX_X + 1;
    P3D->y = rand() % MAX_Y + 1;
    P3D->z = rand() % MAX_Z + 1;
    
    myQueue->send(ID_Point3D, static_cast<Message*>(P3D));
    
    sleep(SLEEP_TIME);
  }
  
  pthread_exit(NULL);
}

void *receiver(void* data)
{
  MsgQueue* myQueue = static_cast<MsgQueue*>(data);
  
  for(;;)
  {
    unsigned long id;
    Message* message = myQueue->receive(id);
    handler(message, id);
    delete message;
  }
  
  pthread_exit(NULL);
}

void handler(Message* msg, unsigned long id)
{
  switch(id)
  {
    case ID_Point3D:
    {
      Point3D* P3D = static_cast<Point3D*>(msg);
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