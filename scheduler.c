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
    int MemS;
};

struct PG_msgbuff
{
    long mtype;
    struct proc Process;
};
 FILE *logptr;
 FILE *perfptr;
 // some variables used inside
 int count = 0;
 int remaining_time;
 int pid;
 int Start_Time = 0;
 bool finish=0;

 // for SJF
 int RunningPID = -1;
 int PreviousCycle = 0;
 int ArrivalTime;
 int FinishedProc = 0;

 // for MLFB
 int CurrentPriority=-1;

 // for Perf
 int Total_Execution_Time = 0;
 int Total_Waiting_Time = 0;
 float Total_WTA = 0;

 struct PCB *ProcessTable;

 struct PCB *pcb = NULL;
 struct PCB *temp = NULL;

 // initialization of ProcessTable


// Data Structures that are initialized when needed according to algorithm
struct Queue *Process_Queue;
struct PQueue *Process_PQueue;
struct Node *node = NULL;
struct Node *tempnode = NULL;
struct PNode *PQ_node;
struct Queue *Process_Queues_Arr[12];

struct Queue* Waiting_List;
struct Node* ListNode;

void finish_SJF();
void finish_HPF();
void finish_RR();
void finish_MQ();
//      make; ./process_generator.out "testcase.txt" -sch 3 -q 1 &
//     make; ./process_generator.out "testcase.txt" -sch 5 -q 2 &
//FOR DEBUGGING : gdb --args ./Process_generator.out -g -sch 3 -q

int main(int argc, char *argv[])
{
 
   if(argc<4){
       printf("argc is less than 4\n");
   }

   signal(SIGUSR2,PG_finish);
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
        Process_Queue = CreateQueue();
    }
    else if (Algorithm_type == 4) // MultilevelQ
    {
    }
    #pragma endregion


    //SJF variables
    int RunningPID = -1;
    int ArrivalTime;
    int PreviousCycle;
    int FinishedProc = 0;

    //some variables used inside
    int count=0;  
    int remaining_time;
    int pid;
    int Start_Time=0;

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
	//perfptr = fopen("scheduler.perf", "w");

    // TODO implement the scheduler :)
    //     1.Start a new process whenever it arrives. (Fork it and give it its parameters)

    // recieve from generator:
    struct PG_msgbuff snt_Process_msg;
    //printf("SchedulerID is %d\n",getpid());

    

    printf("SCHEDULER ID is %d\n",getpid()); 

    while (1 && !finish)
    {

        /* receive all types of messages */
        // rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(snt_Process_msg)-sizeof(PG_msgbuff.mtype), 0, !IPC_NOWAIT);
        //sizeof(long) is the size of mtype
        rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(struct proc), 2, IPC_NOWAIT);
        if (rec_val != -1)
        {   
            printf("id: %d, arrival: %d, runtime:, %d, priority: %d Mem: %d\n",snt_Process_msg.Process.id, snt_Process_msg.Process.ArrivalTime, snt_Process_msg.Process.Runtime, snt_Process_msg.Process.Priority, snt_Process_msg.Process.MemS);
            //in case of message recieval
            //update the PCB with remaining time
            ProcessTable[count].MemS = snt_Process_msg.Process.MemS;
            ProcessTable[count].id = snt_Process_msg.Process.id;

            //remaining_time = snt_Process_msg.Process.Runtime;            
            count++;
        }
        // 2.Switch between two processes according to the scheduling algorithm. (stop
        // the old process and save its state and start/resume another one.)
        if (Algorithm_type == 1) //SJF
        {
            //printf("4) In Algorithm\n");
            if(rec_val!=-1)
            {
                //Add the process in priority queue
                PQ_node = CreatePNode();
                PQ_node->Arrival_Time = snt_Process_msg.Process.ArrivalTime;
                PQ_node->PID = 0;
                PQ_node->id = snt_Process_msg.Process.id;
                PQ_node->Priority = snt_Process_msg.Process.Priority;
                PQ_node->Runtime = snt_Process_msg.Process.Runtime;
                PQ_node->Q_Priority = PQ_node->Runtime;
                EnPQueue(Process_PQueue, PQ_node);
                //printf("5) message Received and enqued with PID = %d\n", PQ_node->PID);

                continue; //go back to the beginning of the loop to check for any process with the same arrival time
            }
            if(RunningPID == -1) //No process is running
            {
                //printf("6) No Process is running\n");
                if(PPeek(Process_PQueue)) //if ready queue is not empty
                {
                    printf("not empty\n");
                    //Dequeue the process
                    PQ_node = DePQueue(Process_PQueue);
                    RunningPID = PQ_node->id;
                    pcb = findPCB(RunningPID, ProcessTable);
                    pcb->state = 1 ; //Running
                    pcb->waiting_time = getClk() - PQ_node->Arrival_Time;
                    pcb->execution_time = PQ_node->Runtime;
                    pcb->remaining_time = PQ_node->Runtime;
/////////////////////////////////////////////////////////////////////////////////////
                    // fork the process
            
                    pid=fork();
                    fflush(stdout);
                        //      make; ./process_generator.out  -sch 3 -q 1 &
                        //    A messing AROUND TEST  make; ./process_generator.out -sch 5 -q 2 &
                    waiter=1;
                    remaining_time = PQ_node->Runtime;
                    if (!pid) // the child joins the cult of processes
                    {

                        //printf("A child was forked\n");

                        my_itoa(remaining_time, bufferion);
                        char *args[] = {"./process.out", bufferion, NULL};

                        
                        execvp(args[0], args);
                    }
                    // system("ps");
                    pcb->pid = pid;
                    //ProcessTable[count].pid = pid;
                    PQ_node->PID = pid;
                    if(waiter){raise(SIGSTOP);}
/////////////////////////////////////////////////////////////////////////////////////    

                    logptr = fopen("scheduler.log", "a");
                    fprintf(logptr,"At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, PQ_node->Runtime, PQ_node->Runtime, pcb->waiting_time);
                    fclose(logptr);

                    //printf("8) At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id,PQ_node->Arrival_Time, PQ_node->Runtime, PQ_node->Runtime, pcb->waiting_time);
                    Total_Execution_Time += PQ_node->Runtime;
                    Total_Waiting_Time += pcb->waiting_time;
                    PreviousCycle = getClk();
                    ArrivalTime = PQ_node->Arrival_Time;     
                    //printf("9) Process with PID = %d, ID = %d started running at time %d\n", PQ_node->PID, pcb->id, getClk());              
                    kill(RunningPID,SIGCONT);

                    if(pcb->remaining_time == 0) //if runtime = 0
                    {
                        //printf("13) Process with PID = %d has finished\n", RunningPID);
                        RunningPID = -1;
                        int TA = getClk() - ArrivalTime;
                        float WTA;
                        if(pcb->execution_time == 0)
                        {
                            WTA = 1.0;
                        }
                        else
                        {
                            WTA = (TA*1.0)/ (pcb->execution_time);
                        }
                        logptr = fopen("scheduler.log", "a");
                        fprintf(logptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, ArrivalTime, pcb->execution_time, 0, 0, TA, WTA);
                        fclose(logptr);
                        //printf("14) At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, ArrivalTime, pcb->execution_time, 0, 0, TA, WTA);
                        Total_WTA += WTA;
                        pcb->execution_time = 0;
                        pcb->remaining_time = 0;
                        pcb->waiting_time = 0;
                        pcb->id = -1;
                        pcb->pid = -1;
                        FinishedProc += 1;
                        if(FinishedProc == Nprocesses)
                        {
                            //printf("15) No more processes! \n");
                            goto barra;
                        }
                    }
                }        
            }
            else //if a process is running decrement the remaining time if a cycle has passed
            {
                //printf("10) A process is running\n");
                if(getClk() == PreviousCycle+1) //1 cycle passed
                {
                    //printf("11) Cycle has passed, now time = %d \n", getClk());
                    PreviousCycle = getClk();
                    pcb = findPCB(RunningPID, ProcessTable);
                    pcb->remaining_time -= 1;
                    if(pcb->remaining_time == 0) //process finished
                    {
                        //printf("13) Process with PID = %d has finished\n", RunningPID);
                        RunningPID = -1;
                        int TA = getClk() - ArrivalTime;
                        float WTA = (TA*1.0)/ (pcb->execution_time);
                        logptr = fopen("scheduler.log", "a");
                        fprintf(logptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, ArrivalTime, pcb->execution_time, 0, 0, TA, WTA);
                        fclose(logptr);
                        //printf("14) At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, ArrivalTime, pcb->execution_time, 0, 0, TA, WTA);
                        Total_WTA += WTA;
                        pcb->execution_time = 0;
                        pcb->remaining_time = 0;
                        pcb->waiting_time = 0;
                        pcb->id = -1;
                        pcb->pid = -1;
                        FinishedProc += 1;
                        if(FinishedProc == Nprocesses)
                        {
                            //printf("15) No more processes! \n");
                            goto barra;
                        }
                    }
                }
            }
        }
        else if (Algorithm_type == 2)//HPF
        {
             if(rec_val!=-1)
            {
                //intialize the PQ_Node
                printf("I recieved a message\n");
                snt_Process_msg.Process.SchPriority=snt_Process_msg.Process.Priority;
                // struct Node*node=malloc(sizeof(struct Node));
                PQ_node=CreatePNode();
                PQ_node->PID=pid;
                PQ_node->Priority=snt_Process_msg.Process.Priority;
                PQ_node->Q_Priority=snt_Process_msg.Process.Priority;
                PQ_node->Arrival_Time=snt_Process_msg.Process.ArrivalTime;
                PQ_node->Runtime=snt_Process_msg.Process.Runtime;
                // struct proc*proc=malloc(sizeof(struct proc));
                // *proc=snt_Process_msg.Process;
                // node->process=proc;
                EnPQueue(Process_PQueue,PQ_node);
                pcb=findPCB(PQ_node->PID,ProcessTable);
                if(!(Process_PQueue->Header->Next))
                {
                    pcb->state=1;
                    kill(pcb->pid,SIGCONT);
                    printf("I sent to process %d\n",pcb->id);
                }
                else if(!(pcb->state))
                {
                    //system("ps&");
                    printf("Context Switch\n");
                    pid=Process_PQueue->Header->Next->PID;
                    struct PCB*temp=findPCB(pid,ProcessTable);
                    temp->state=0;
                    kill(temp->id,SIGSTOP);
                    //headpcb->waiting_time=(getClk())-(headpcb->arrival_time)-(headpcb->execution_time)+(headpcb->remaining_time);
                    pcb->state=1;
                    kill(pcb->id,SIGCONT);
                }
            }
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
                        //logptr = fopen("scheduler.log", "a");
                        fprintf(logptr,"At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                        //fclose(logptr);
                        printf("At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                    }
                    else
                    {
                        //logptr = fopen("scheduler.log", "a");
                        fprintf(logptr,"At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                        //fclose(logptr);
                        printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
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
                //kill(pid, SIGSTOP); // SIGSTOP=19
                node = Dequeue(Process_Queue);
                // CALCULATE THINGS in PCB
                pcb->remaining_time -= quantum;
                pcb->execution_time += quantum;
                pcb->state = 0;

                Total_Execution_Time += quantum;

                if (pcb->remaining_time <= 0)
                {
                    ; // Output & Delete
                    //logptr = fopen("scheduler.log", "a");
                    fprintf(logptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time, getClk() - node->Arrival_Time, (float)(getClk() - node->Arrival_Time) / pcb->execution_time);
                    //fclose(logptr);
                    printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time, getClk() - node->Arrival_Time, (float)(getClk() - node->Arrival_Time) / pcb->execution_time);
                    Total_WTA += (float)(getClk() - node->Arrival_Time) / pcb->execution_time;
                    // Delete data
                    pcb->execution_time = 0;
                    pcb->remaining_time = 0;
                    pcb->waiting_time = 0;
                    pcb->id = -1;
                    pcb->pid = -1;
                    node=NULL;
                }
                else
                {
                    //logptr = fopen("scheduler.log", "a");
                    fprintf(logptr, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                    //fclose(logptr);
                    Enqueue(Process_Queue, node);
                    node=NULL;
                }

                if (isPCBempty(ProcessTable) && PG_fanoosh)
                {
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
    perfptr = fopen("scheduler.perf", "w");
    //printf("16) I went barra \n");
    fprintf(perfptr,"CPU utilization = %.2f%%Avg\n",(float)(Total_Execution_Time*100/getClk()));    
    fprintf(perfptr,"WTA=%.2f\n",(float)(Total_WTA/Nprocesses));    
    fprintf(perfptr,"Avg Waiting = %.2f\n",(float)(Total_Waiting_Time/Nprocesses));    
    //printf("17) CPU utilization = %.2f%%Avg\n", (float)(Total_Execution_Time*100/getClk()));
    //printf("18) WTA=%.2f\n",(float)(Total_WTA/Nprocesses));
    //printf("19) Avg Waiting = %.2f\n",(float)(Total_Waiting_Time/Nprocesses)); 
    //fclose(logptr);
    fclose(perfptr);
    // upon termination release the clock resources.
    destroyClk(true);


}
