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

struct Mem_Node
{
    struct Mem_Node* Next;
    int start;
}Mem_def={NULL, -1};

struct Mem_List
{
    struct Mem_Node* Header;
};


struct Mem_Node* CreateMem_Node()
{
    struct Mem_Node *Temp = (struct Mem_Node*)malloc(sizeof(struct Mem_Node));
    *Temp = Mem_def;
    return Temp;
}

struct Mem_List* CreateMem_List()
{
    struct Mem_List *Temp = (struct Mem_List *)malloc(sizeof(struct Mem_List));
    Temp->Header = NULL;
    return Temp;
}

void Insert(struct Mem_List *Q ,struct Mem_Node *NewNode)
{
    struct Mem_Node *Temp = NewNode;
    if(Q->Header == NULL) //If Empty
    {
        Q->Header = Temp;
        Q->Header->Next = NULL;
        return;
    }

    //if address of head is greater than the new node
    struct Mem_Node *ptr = Q->Header;
    if(ptr->start > Temp->start)
    {
        Temp->Next = Q->Header;
        Q->Header = Temp;
        return;
    }
    //if not empty, loop till you reach the right position
    while ((ptr->Next != NULL) && ptr->Next->start < Temp->start)
    {   
        ptr = ptr->Next;
    }
    //either add at the end of the queue or in the chosen position
    Temp->Next = ptr->Next;
    ptr->Next = Temp;
}

void Delete(struct Mem_List *Q, int S)
{
    //If empty
    if(!Q->Header)
    {
        return;
    }
    //If not empty
    struct Mem_Node *ptr = Q->Header;
    if(Q->Header->start == S)
    {
        struct Mem_Node *Temp = Q->Header;
        Q->Header = Temp->Next;
        free(Temp);
        return;
    }
    else
    {
        while (ptr->Next && ptr->Next->start != S)
        {
           ptr = ptr->Next;
        }
        if(!ptr->Next)
        {   
            //printf("list empty\n");
            return;
        }
        struct Mem_Node *Temp = ptr->Next;
        ptr->Next = Temp->Next;
        free(Temp);
    }
    
}

bool IsEmptyList(struct Mem_List *Q)
{
    if(Q->Header->start == -1)
    {
        return true;
    }
    return false;
}

void printList(struct Mem_List *Q)
{
    //If empty
    if(!Q->Header)
    {
        printf("List is empty\n");
    }
    //If not empty
    struct Mem_Node *ptr = Q->Header;
    while(ptr)
    {
        printf("%d-", ptr->start);
        ptr = ptr->Next;
    }
    printf("\n");
}