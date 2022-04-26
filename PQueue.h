#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>

typedef short bool;


struct PNode 
{
    struct PNode *Next;
    int Q_Priority; //different from one algorithm to another
    //Input File Data
    int PID;
    int Priority;
    int Arrival_Time;
    int Runtime;
};


struct PQueue
{
    struct PNode *Header;
};

void EnPQueue(struct PQueue *Q ,struct PNode *NewPNode)
{
    struct PNode *Temp = NewPNode;
    if(Q->Header == NULL) //If Empty
    {
        Q->Header = Temp;
        Q->Header->Next = NULL;
        return;
    }

    //if priority of head is greater than the new node
    struct PNode *ptr = Q->Header;
    if(ptr->Q_Priority > Temp->Q_Priority)
    {
        Temp->Next = Q->Header;
        Q->Header = Temp;
        return;
    }
    //if not empty, loop till you reach the right position
    while ((ptr->Next != NULL) && ptr->Next->Q_Priority <= Temp->Q_Priority)
    {   
        ptr = ptr->Next;
    }
    //either add at the end of the queue or in the chosen position
    Temp->Next = ptr->Next;
    ptr->Next = Temp;
}

struct PNode* DePQueue(struct PQueue *Q)
{
    //If empty
    if(Q->Header == NULL)
    {
        return NULL;
    }
    //If not empty
    struct PNode *Temp = Q->Header;
    Q->Header = Q->Header->Next;
    return Temp;
}

struct PNode* PPeek(struct PQueue *Q)
{
    //If empty
    if(!Q->Header)
    {
        return NULL;
    }
    struct PNode *Temp = Q->Header;
    return Temp;
}

struct PNode* CreatePNode()
{
    struct PNode *Temp = (struct PNode*)malloc(sizeof(struct PNode));
    Temp->Next = NULL;
    return Temp;
}

struct PQueue* CreatePQueue()
{
    struct PQueue *Temp = (struct PQueue *)malloc(sizeof(struct PQueue));
    Temp->Header = NULL;
    return Temp;
}