#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <vector>
#include "msgqueue.h"
#include "msgtypes.h"

using namespace std;

// Constants
const int CAR_AMOUNT    = 5;            // Amount of car threads
const int SLEEP_TIME    = 10;           // Maximum sleeping time (seconds)
const int PARKING_SPOTS = 3;            // Amount of spots in the parking lot
const int QUEUE_SIZE    = CAR_AMOUNT;   // Queue size for the MsgQueues

enum
{  
    ID_ENTRY_OPEN_REQ,
    ID_ENTRY_OPEN_CFM,
    ID_ENTRY_NEW_SPOT,
    ID_EXIT_OPEN_REQ,
    ID_EXIT_OPEN_CFM,
    ID_EXIT_OUTSIDE_IND,
    ID_INTERN_QUEUE
};

// Struct containing msgqueues and carID for a car.
struct MsgQueuePack
{
    MsgQueue* carMsgQueue;
    MsgQueue* entryMsgQueue;
    MsgQueue* exitMsgQueue;
    unsigned int carID;
};

// Struct containing MsgQueues required for exit
struct GuardQueuePack
{
    MsgQueue* entryMsgQueue;
    MsgQueue* exitMsgQueue;
};

void carHandler(Message* msg, unsigned long id, MsgQueuePack* mqPack)
{
    switch(id)
    {
        case ID_ENTRY_OPEN_CFM:
        {
            // Now sleeping
            usleep(rand()%SLEEP_TIME*1000000+1);

            // Trying to exit now
            ExitOpenReq* Req = new ExitOpenReq;
            Req->carID = mqPack->carID;
            Req->msgQueue = mqPack->carMsgQueue;

            mqPack->exitMsgQueue->send(ID_EXIT_OPEN_REQ, static_cast<Message*>(Req));
            break;
        }
        case ID_EXIT_OPEN_CFM:
        {
            ExitOutsideInd* Ind = new ExitOutsideInd;
            Ind->carID = mqPack->carID;
            mqPack->exitMsgQueue->send(ID_EXIT_OUTSIDE_IND, static_cast<Message*>(Ind));

            // Now sleeping
            usleep(rand()%SLEEP_TIME*1000000+1);

            // Trying to re-enter now
            EntryOpenReq* Req = new EntryOpenReq;
            Req->carID = mqPack->carID;
            Req->msgQueue = mqPack->carMsgQueue;

            mqPack->entryMsgQueue->send(ID_ENTRY_OPEN_REQ, static_cast<Message*>(Req));
            break;
        }
        default:
            cout << "Handler received unknown identifier: " << id << endl;
    }
}



void entryHandler(Message* msg, unsigned long id, MsgQueue* entryMsgQueue)
{
    static int parkingSpots = PARKING_SPOTS;
    static int carsWaiting = 0;
    // interQueue should always be able to hold all cars waiting
    // (therefore queue size is CAR_AMOUNT)
    static MsgQueue internQueue(CAR_AMOUNT);

    switch(id)
    {
        case ID_ENTRY_OPEN_REQ:
        {
            EntryOpenReq* Req = static_cast<EntryOpenReq*>(msg);

            if (parkingSpots > 0)
            {
                parkingSpots--;
                EntryOpenCfm* Cfm = new EntryOpenCfm;
                cout << "+ Entry Guard: Now letting car " << Req->carID << " in."
                     << " (" << parkingSpots << " spots left)" << endl;
                Req->msgQueue->send(ID_ENTRY_OPEN_CFM, static_cast<Message*>(Cfm));
            }
            else
            {
                carsWaiting++;
                InternQueueReq* internReq = new InternQueueReq;
                internReq->carID = Req->carID;
                internReq->msgQueue = Req->msgQueue;
                cout << "  Entry Guard: Car " << Req->carID << " must wait for a spot." << endl;
                internQueue.send(ID_INTERN_QUEUE, internReq);
            }
            break;
        }

        case ID_ENTRY_NEW_SPOT:
        {
            parkingSpots++;
            cout << "  Entry Guard: Message from Exit Guard - Car has left."
                 << " (" << parkingSpots << " spots left)" << endl;
            if (carsWaiting > 0)
            {
                carsWaiting--;
                // Read from internal
                unsigned long id;
                Message* msg = internQueue.receive(id);
                EntryOpenReq* Req = static_cast<EntryOpenReq*>(msg);

                // Send new message
                EntryOpenReq* newReq = new EntryOpenReq;
                newReq->msgQueue = Req->msgQueue;
                newReq->carID = Req->carID;

                entryMsgQueue->send(ID_ENTRY_OPEN_REQ, static_cast<Message*>(newReq));
            }
            break;
        }
        default:
            cout << "Handler received unknown identifier: " << id << endl;
    }
}

void exitHandler(Message* msg, unsigned long id, MsgQueue* entryMsgQueue)
{
    switch(id)
    {
        case ID_EXIT_OPEN_REQ:
        {
            ExitOpenReq* Req = static_cast<ExitOpenReq*>(msg);
            ExitOpenCfm* Cfm = new ExitOpenCfm;
            cout << "- Exit Guard: Now letting car " << Req->carID << " out." << endl;
            Req->msgQueue->send(ID_EXIT_OPEN_CFM, static_cast<Message*>(Cfm));
            break;
        }

        case ID_EXIT_OUTSIDE_IND:
        {
            EntryNewSpotInd* Ind = new EntryNewSpotInd;
            entryMsgQueue->send(ID_ENTRY_NEW_SPOT, static_cast<Message*>(Ind));
            break;
        }
        default:
            cout << "Handler received unknown identifier: " << id << endl;
    }
}

void *carFunc(void* arg)
{
    MsgQueuePack* mqPack = static_cast<MsgQueuePack*>(arg);
    for(;;)
    {
        unsigned long id;
        Message* msg = mqPack->carMsgQueue->receive(id);
        carHandler(msg, id, mqPack);
        delete msg;
    }
    pthread_exit(NULL);
}

void *entryFunc(void* arg)
{
    MsgQueue* mq = static_cast<MsgQueue*>(arg);

    for(;;)
    {
        unsigned long id;
        Message* msg = mq->receive(id);
        entryHandler(msg, id, mq);
        delete msg;
    }
    pthread_exit(NULL);
}

void *exitFunc(void* arg)
{
    GuardQueuePack* mq = static_cast<GuardQueuePack*>(arg);

    for(;;)
    {
        unsigned long id;
        Message* msg = mq->exitMsgQueue->receive(id);
        exitHandler(msg, id, mq->entryMsgQueue);
        delete msg;
    }

    pthread_exit(NULL);
}

int main()
{
    cout << "PLCS started." << endl;

    // Error variable
    int err = 0;

	// Seed rand() with time()
	srand (time(NULL));

    // Entry and exit queue
    MsgQueue entryMsgQueue(QUEUE_SIZE);
    MsgQueue exitMsgQueue(QUEUE_SIZE);

    vector<MsgQueue> carMsgQueue(CAR_AMOUNT, MsgQueue(QUEUE_SIZE));

    // Generating a msgQueuePack for each car
    MsgQueuePack msgQueuePack[CAR_AMOUNT];
    for(int i = 0; i < CAR_AMOUNT; i++)
    {
        msgQueuePack[i].carID = i+1;    // Car ID starts at 1 not 0
        msgQueuePack[i].carMsgQueue = &carMsgQueue[i];
        msgQueuePack[i].entryMsgQueue = &entryMsgQueue;
        msgQueuePack[i].exitMsgQueue = &exitMsgQueue;
    }

    GuardQueuePack guardMsgPack;
    guardMsgPack.entryMsgQueue = &entryMsgQueue;
    guardMsgPack.exitMsgQueue = &exitMsgQueue;

    // Initializing entry and exit threads
    pthread_t entryThread, exitThread;

    if ( (err = pthread_create(&entryThread, NULL, entryFunc, static_cast<void*>(&entryMsgQueue))) )
    {
        cout << "Could not create entryThread, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    if ( (err = pthread_create(&exitThread, NULL, exitFunc, static_cast<void*>(&guardMsgPack))) )
    {
        cout << "Could not create exitThread, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }

    // Initializing car threads
    pthread_t carThread[CAR_AMOUNT];
    for (int i = 0; i < CAR_AMOUNT; i++)
    {
        if ( (err = pthread_create(&carThread[i], NULL, carFunc, static_cast<void*>(&msgQueuePack[i]))) )
        {
            cout << "Could not create carThread with ID " << i << ", ERROR: " << err << endl;
            return EXIT_FAILURE;
        }
    }

    // Starting the chain-reaction
    for(int i = 0; i < CAR_AMOUNT; i++)
    {
        EntryOpenReq* Req = new EntryOpenReq;
        Req->carID = i+1;
        Req->msgQueue = &carMsgQueue[i];
        entryMsgQueue.send(ID_ENTRY_OPEN_REQ, Req);
        // Sleeping some time between each car start
        usleep(rand()%SLEEP_TIME*100000+1);
    }

    // Join pthreads
    if ( (err = pthread_join(entryThread, NULL)) )
    {
        cout << "Could not join entryThread, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    if ( (err = pthread_join(exitThread, NULL)) )
    {
        cout << "Could not join exitThread, ERROR: " << err << endl;
        return EXIT_FAILURE;
    }
    for (int i = 0; i < CAR_AMOUNT; i++)
    {
        if ( (err = pthread_join(carThread[i], NULL)) )
        {
            cout << "Could not join carThread with ID " << i << ", ERROR: " << err << endl;
            return EXIT_FAILURE;
        }
    }

    cout << "PLCS ended." << endl;
    return EXIT_SUCCESS;
}


