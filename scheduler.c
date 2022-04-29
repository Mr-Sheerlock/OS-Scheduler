#include "headers.h"

struct PCB
{
    bool state; // 0 for waiting, 1 for running
    int remaining_time;
    int waiting_time;
    int execution_time;
    int id;
    int pid;
}PCB_def={0,0,0,0,-1,-1};

struct proc
{
    int id;
    int ArrivalTime;
    int Runtime;
    int Priority;
    int SchPriority;
};

struct PG_msgbuff
{
    long mtype;
    struct proc Process;
};

int Nprocesses;   


struct PCB* findPCB(int pid, struct PCB* ProcessTable){
    for(int i=0;i<Nprocesses;i++){
        if(pid== ProcessTable[i].pid){
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


//      make; ./process_generator.out testcase.txt -sch 3 -q 1 &
//     make; ./process_generator.out testcase.txt -sch 5 -q 2 &

int main(int argc, char *argv[])
{
 
   if(argc<4){
       printf("argc is less than 4\n");
   }
    // initialization from ProcessGenerator:
    int Algorithm_type= atoi(argv[1]);   // 1 SFJ 2 HPF 3 RR 4 MultilevelQ
    int quantum=atoi(argv[2]);
    Nprocesses=atoi(argv[3]);

    initClk();


    //Process Table
    struct PCB ProcessTable[Nprocesses];
    struct PCB* pcb;

    //initialization of ProcessTable
    for(int i=0; i<Nprocesses;i++){
        ProcessTable[i]=PCB_def;
    }
    

    //Data Structures that are initialized when needed according to algorithm
    struct Queue* Process_Queue;
    struct PQueue* Process_PQueue;
    // struct CQueue* Process_Queue; // for RR
    struct Node *node;
    struct PNode *PQ_node;
    
    #pragma region DS INIT
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
        Process_Queue = Process_Queue();
    }
    else if (Algorithm_type == 4) // MultilevelQ
    {
    }
    #pragma endregion
     
    //some variables used inside
    int count=0;  
    int remaining_time;
    int pid;
    int Start_Time=0;
    bool flag=0;

    //for Perf
    int Total_Execution_Time=0;
    int Total_Waiting_Time=0;
    float Total_WTA=0;
    // for Process Generator Communication
    key_t key_id;
    int rec_val, msgq_id;
    key_id = ftok("dummy", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    //for Process.c initial Communication
    char bufferion[20]; //buffer for integer to char conversion
    
    //For Output
    FILE *logptr;
    FILE *perfptr;
	logptr = fopen("scheduler.log", "w");
	perfptr = fopen("scheduler.perf", "w");

    // TODO implement the scheduler :)
    //     1.Start a new process whenever it arrives. (Fork it and give it its parameters)

    // recieve from generator:
    struct PG_msgbuff snt_Process_msg;
    printf("SchedulerID is %d\n",getpid());

    while (1)
    {

        /* receive all types of messages */
        // rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(snt_Process_msg)-sizeof(PG_msgbuff.mtype), 0, !IPC_NOWAIT);
        //sizeof(long) is the size of mtype
        rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(struct proc), 2, IPC_NOWAIT);
        if (rec_val != -1)
        {   
            //in case of message recieval
           
            remaining_time = snt_Process_msg.Process.Runtime;
            //update the PCB with remaining time
            ProcessTable[count].remaining_time = remaining_time;
            ProcessTable[count].id = snt_Process_msg.Process.id;

            // fork the process
            
            // fflush(stdout);
            int lol=fork();
            fflush(stdout);
                        //      make; ./process_generator.out  -sch 3 -q 1 &
                        //    A messing AROUND TEST  make; ./process_generator.out -sch 5 -q 2 &

            if (!lol) // the child joins the cult of processes
            {

                printf("A child was forked\n");

                my_itoa(remaining_time, bufferion);
                char *args[] = {"./process.out", bufferion, NULL};

                execvp(args[0], args);
            }
            
            // immediately put the child to good sleep C:
            // printf("before the kill ");
            // printf("pid is %d\n", getpid());
            ProcessTable[count].pid = lol;
            count++;
            pid = lol;
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
            // pid=lol;
            // printf("PS MN ta7t\n");
            // system("ps &");
            // printf("\nmy pid is %d\n",getpid());
            if(rec_val!=-1){
                node=CreateNode();
                node->PID=pid;
                node->Priority=snt_Process_msg.Process.Priority;
                node->Arrival_Time=snt_Process_msg.Process.ArrivalTime;
                node->Runtime=snt_Process_msg.Process.Runtime;
                Enqueue(Process_Queue,node);
                node=NULL;
            }

            if(node==NULL){
                node=Peek(Process_Queue);
            }

            if(node!=NULL){
                
                pid=node->PID;
                pcb=findPCB(pid, ProcessTable); //4a8ala sa7
                // printf("PCB value right now is id %d pid %d execution time %d\n", pcb->id,pcb->pid,pcb->execution_time);
                //how to get waiting???? 

                //fl bdaya 5ales 
                if(pcb->state==0){
                    pcb->waiting_time+=(getClk()-node->Arrival_Time) -pcb->execution_time;
                    Total_Waiting_Time+=pcb->waiting_time;
                    Start_Time=getClk();
                    // printf("Starting time is %d \n",Start_Time);
                    if (pcb->remaining_time==node->Runtime)
                    {
                        fprintf(logptr,"At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                        printf("At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                    }
                    else
                    {
                        fprintf(logptr,"At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                        printf("At time %d process %d resumed arr %d total %d remain %d wait %d", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                    }
                }
                // printf("NODE IS NOT NULL YA GAMA3A!\n");
                kill(pid,SIGCONT);// SIGCONT=18

                pcb->state=1;
            }

            
            //output first


            
            // sleep(quantum);
            // system("ps &");

            //law 3adda el wa2t
            if ((getClk()- Start_Time) == quantum && node!=NULL)
            {
                system("ps &");
                printf("Ana f3lan 5alast\n");
                kill(pid, SIGSTOP); // SIGSTOP=19
                node = Dequeue(Process_Queue);
                // CALCULATE THINGS in PCB
                pcb->remaining_time -= quantum;
                pcb->execution_time += quantum;
                pcb->state = 0;

                Total_Execution_Time += quantum;

                if (pcb->remaining_time <= 0)
                {
                    ; // Output & Delete
                    fprintf(logptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time, getClk() - node->Arrival_Time, (float)(getClk() - node->Arrival_Time) / pcb->execution_time);
                    Total_WTA += (float)(getClk() - node->Arrival_Time) / pcb->execution_time;
                    // Delete data
                    pcb->execution_time = 0;
                    pcb->remaining_time = 0;
                    pcb->waiting_time = 0;
                    pcb->id = -1;
                    pcb->pid = -1;
                }
                else
                {
                    fprintf(logptr, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                    Enqueue(Process_Queue, node);
                }

                if (isPCBempty(ProcessTable) && flag)
                {
                    printf("I Should be going to: \n");
                    goto barra;
                }
            }



            
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

        // 5.Report the following information:
        // a) CPU utilization.
        // b) Average Weighted Turnaround Time 
        // c) Average Waiting Time 
        
        

    }

    // 6.Generate two files: (check the input/output section
    // below)(a)Scheduler.log

    // (b) Scheduler.perf 
    barra:
    printf("I WENT BARRA!\n");

    fprintf(perfptr,"CPU utilization = %.2f%%Avg\n",(float)(Total_Execution_Time/getClk()));    
    fprintf(perfptr,"WTA=%.2f\n",(float)(Total_WTA/Nprocesses));    
    fprintf(perfptr,"Avg Waiting = %.2f\n",(float)(Total_Waiting_Time/Nprocesses));    

    fclose(logptr);
    fclose(perfptr);
    // upon termination release the clock resources.
    destroyClk(true);


}
