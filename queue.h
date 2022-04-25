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
#define true 1
#define false 0

struct Node 
{
    struct Node *Next;
    //Input File Data
    int Priority;
    int ID;
    int Arrival_Time;
    int Runtime;
};


struct Queue
{
    struct Node *Header;
};

void Enqueue(struct Queue *Q ,struct Node *NewNode)
{
    struct Node *Temp = NewNode;
    Temp->Next = NULL;
    if(Q->Header == NULL) //If Empty
    {
        Q->Header = Temp;
        //Q->Header->Next = NULL;
        return;
    }

    //if not empty, loop till you reach the end of the queue
    struct Node *ptr = Q->Header;
    while(ptr->Next != NULL)
    {
        ptr = ptr->Next;
    }
    ptr->Next = Temp;
}

struct Node* Dequeue(struct Queue *Q)
{
    //If empty
    if(!Q->Header)
    {
        return NULL;
    }
    //If not empty
    struct Node *Temp = Q->Header;
    Q->Header = Temp->Next;
    return Temp;
}

struct Node* Peek(struct Queue *Q)
{
    //If empty
    if(!Q->Header)
    {
        return NULL;
    }
    struct Node *Temp = Q->Header;
    return Temp;
}

struct Node* CreateNode()
{
    struct Node *Temp = (struct Node*)malloc(sizeof(struct Node));
    Temp->Next = NULL;
    return Temp;
}

struct Queue* CreateQueue()
{
    struct Queue *Temp = (struct Queue *)malloc(sizeof(struct Queue));
    Temp->Header = NULL;
    return Temp;
}