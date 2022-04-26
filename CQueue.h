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



struct Node;



struct CQueue
{
    struct Node *Header;
};

void CEnqueue(struct CQueue *Q ,struct Node *NewNode)
{
    struct Node *Temp = NewNode;
    Temp->Next = Q->Header;
    if(Q->Header == NULL) //If Empty
    {
        Q->Header = Temp;
        //Q->Header->Next = NULL;
        return;
    }

    //if not empty, loop till you reach the end of the CQueue
    struct Node *ptr = Q->Header;
    while(ptr->Next != Q->Header)
    {
        ptr = ptr->Next;
    }
    ptr->Next = Temp;
}

struct Node* CDequeue(struct CQueue *Q)
{
    //If empty
    if(!Q->Header)
    {
        return NULL;
    }
    //If not empty
    struct Node *Temp = Q->Header;

    //loop till you reach the end of the CQueue
    while(Temp->Next != Q->Header)
    {
        Temp = Temp->Next;
    }
    Temp->Next = Q->Header->Next;

    Q->Header = Temp->Next;
    return Temp;
}

struct Node* CPeek(struct CQueue *Q)
{
    //If empty
    if(!Q->Header)
    {
        return NULL;
    }
    struct Node *Temp = Q->Header;
    return Temp;
}


struct CQueue* CreateCQueue()
{
    struct CQueue *Temp = (struct CQueue *)malloc(sizeof(struct CQueue));
    Temp->Header = NULL;
    return Temp;
}