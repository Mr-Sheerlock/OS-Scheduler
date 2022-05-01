#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
struct PCB
{
    int state; // 0 for waiting, 1 for running
    int remaining_time;
    int waiting_time;
    int execution_time;
    int arrival_time;
    int id;
}PCB_def={0,0,0,0,-1};
struct PCB* findPCB(int id, struct PCB* pcb,int Nprocesses){
    for(int i=0;i<Nprocesses;i++){
        if(id== pcb[i].id){
            return &pcb[i];
        }
    }
    printf("not found\n");
    return NULL;
}
struct proc{
    int pid;
    int id;
    int ArrivalTime;
    int RunTime;
    int Priority;
    int SchPriority;  // To Be Assigned in Scheduler Based on Algorithm
};
struct msg{
    long mtype;
    struct proc Process;
};
struct Node{
    struct proc *process;
    struct Node*Next;
}Node_def={NULL,NULL};
struct List{
    struct Node*Head;
    int count;
}List_def={NULL,0};
// Priority Queue Enqueue and Dequeue Functions
struct Node* dequeue(struct List*list)
{
    if(!list->Head){return NULL;}
    struct Node* temp=list->Head;
    list->Head=list->Head->Next;
    return temp;
}
void enqueue(struct List*list,struct Node* node)
{
    if(!list->Head){list->Head=node;list->count++;return;}
    if(node->process->SchPriority<=list->Head->process->SchPriority)
    {
        node->Next=list->Head;
        list->Head=node;
        list->count++;
        return;
    }
    struct Node * tempptr=list->Head;
    do
    {
        if(!tempptr->Next){tempptr->Next=node;list->count++;return;}
        if(node->process->SchPriority<=tempptr->Next->process->SchPriority)
        {
            node->Next=tempptr->Next;
            tempptr->Next=node;
            list->count++;
            return;
        }
    } while (tempptr=tempptr->Next);
    
}