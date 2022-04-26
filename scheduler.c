#include "headers.h"

struct PCB
{
    bool state; // 0 for waiting, 1 for running
    int remaining_time;
    int waiting_time;
    int execution_time;

}PCB_def={0,0,0,0};

struct proc
{
    int pid;
    int ArrivalTime;
    int Runtime;
    int Priority;
};

struct PG_msgbuff
{
    long mtype;
    struct proc Process;
};

int main(int argc, char *argv[])
{

    initClk();
    char bufferion[20];
    // initialization that is expected from ProcessGenerator:
    int Algorithm_type = 3; // atoi(argv[0]);   // 1 SFJ 2 HPF 3 RR 4 MultilevelQ
    int Nprocesses = 3;     // atoi(argv[1]);
    int quantum=2; //atoi(argv[2])

    // int PIDArr[Nprocesses]; //MIGHT BE REPLACED BY A QUEUE
    struct PCB PCBArr[Nprocesses];

    for (int i = 0; i < Nprocesses; i++)
    {
        PCBArr[i].state = 0; // waiting by default
        PCBArr[i].execution_time = 0;
        PCBArr[i].waiting_time = 0;
    }

    // Initialize when needed
    struct Queue* Process_Queue;
    struct PQueue* Process_PQueue;
    struct CQueue* Process_CQueue; // for RR

    struct Node *node;
    struct PNode *PQ_node;

    //THIS PART IS PG responsibility
    if (Algorithm_type == 1) // SJF
    {
        Process_PQueue = CreatePQueue();
    }
    else if (Algorithm_type == 2) // HPF
    {
        Process_PQueue = CreatePQueue();
    }
    else if (Algorithm_type == 3) // RR
    {
        Process_CQueue = CreateCQueue();
    }
    else if (Algorithm_type == 4) // MultilevelQ
    {
    }

    int count;
    int remaining_time;
    int pid;
    // for Process Generator Communication
    key_t key_id;
    int rec_val, msgq_id;

    key_id = ftok("dummy", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    // TODO implement the scheduler :)

    //     1.Start a new process whenever it arrives. (Fork it and give it its parameters)

    // recieve from generator:
    struct PG_msgbuff snt_Process_msg;
    while (1)
    {

        /* receive all types of messages */
        // rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(snt_Process_msg)-sizeof(PG_msgbuff.mtype), 0, !IPC_NOWAIT);
        //sizeof(long) is the size of mtype
        rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(snt_Process_msg) - sizeof(long), snt_Process_msg.mtype, !IPC_NOWAIT);

        if (rec_val == -1)
            ; // pass
        else
        {
            // calculate remaining time for process
            if (Algorithm_type < 3)
            {
                remaining_time = snt_Process_msg.Process.Runtime;
            }
            else if (Algorithm_type == 3)
            {
                // remaining_time = snt_Process_msg.Runtime + 7aga;
            }
            else
            {
                // remaining_time = snt_Process_msg.Runtime+7aga akeed akbar;
            }
            //update the PCB with remaining time
            PCBArr[count].remaining_time = remaining_time;

            // fork the process
            // PIDArr[count] = fork();
            pid=fork();
            if (pid == 0) //the child joins the cult of processes
            {
                // calculate remaining time:
                char *args[] = {"./process", my_itoa(remaining_time,bufferion), NULL};
                execvp(args[0], args);
            }
            //immediately put the child to good sleep C: 
            kill(pid,SIGSTOP); // SIGSTP=19
        }
        // 2.Switch between two processes according to the scheduling algorithm. (stop
        // the old process and save its state and start/resume another one.)
        if (Algorithm_type == 1) //SJF
        {
            
        }
        else if (Algorithm_type == 2)//HPF
        {
            
        }
        else if (Algorithm_type == 3)//RR
        {
            //if recieved a message, create a new node for process
            if(rec_val!=-1){
                node=CreateNode();
                node->PID=pid;
                node->Priority=snt_Process_msg.Process.Priority;
                node->Arrival_Time=snt_Process_msg.Process.ArrivalTime;
                node->Runtime=snt_Process_msg.Process.Runtime;
                CEnqueue(Process_CQueue,node);
            }

            node=CDequeue(Process_CQueue);
            pid=node->PID;

            //CALCULATE THINGS in PCB
            kill(pid,SIGCONT);// SIGCONT=18

            sleep(quantum);
            
            kill(pid,SIGSTOP);// SIGSTOP=19
            //CALCULATE THINGS in PCB

        }
        else if (Algorithm_type == 4)//MultilevelQ
        {
            
        }

        // 3.Keep a process control block (PCB) for each process in the system. A PCB
        // should keep track of the state of a process; running/waiting, execution time,
        // remaining time, waiting time, etc. --done

        //4.Delete the data of the process when it finishes its job. The scheduler knows
        // that a process has finished if its remaining time reaches zero. Note that the
        // scheduler does NOT terminate the process. It just deletes its data from the
        // process control block and its data structures.
    }

    // msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);

    // upon termination release the clock resources.

    destroyClk(true);
}
