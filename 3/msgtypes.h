#pragma once
#include "msgqueue.h"

struct EntryOpenReq : public Message
{
    MsgQueue* msgQueue;
    unsigned long carID;
};

struct EntryOpenCfm: public Message
{
};

struct ExitOpenReq : public Message
{
    MsgQueue* msgQueue;
    unsigned long carID;
};

struct ExitOpenCfm: public Message
{
};

struct ExitOutsideInd : public Message
{
    unsigned long carID;
};

struct EntryNewSpotInd: public Message
{
    unsigned long carID;
};

struct InternQueueReq : public Message
{
    MsgQueue* msgQueue;
    unsigned long carID;
};
