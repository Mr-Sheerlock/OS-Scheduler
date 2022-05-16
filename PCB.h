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

#define true 1
#define false 0


int Nprocesses;   

struct PCB
{
    bool state; // 0 for waiting, 1 for running
    int remaining_time;
    int waiting_time;
    int execution_time;
    int id;
    int pid;
    int MStart;
    int MEnd;
    int MemS;
}PCB_def={0,0,0,0,-1,-1,-1, -1, 0};

struct PCB* findPCB(int pid, struct PCB* ProcessTable){
    for(int i=0;i<Nprocesses;i++){
        if(pid== ProcessTable[i].id){
            return &ProcessTable[i];
        }
    }
    return NULL;
}

bool isPCBempty(struct PCB* pcb){

    for (int i=0;i<Nprocesses;i++){
        if(pcb[i].pid!=-1){
            return false;
        }
    }
    return true;
}

void updateWaiting(struct PCB* pcb){
    for (int i=0;i<Nprocesses;i++){
        if(pcb[i].pid!=-1&&pcb[i].state==0)
            pcb[i].waiting_time+=1;
    }
}
