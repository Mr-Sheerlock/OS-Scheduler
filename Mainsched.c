#include "headers.h"
int waiter=1;
void cont()
{
    waiter=0;
} 

struct proc
{
    int pid;
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

void finish_SJF();
void finish_HPF();
void finish_RR();
void finish_MQ();
//      make; ./process_generator.out "testcase.txt" -sch 3 -q 1 &
//     make; ./process_generator.out "testcase.txt" -sch 5 -q 2 &
//FOR DEBUGGING : gdb --args ./Process_generator.out -g -sch 3 -q

int main(int argc, char *argv[])
{
    signal(SIGCONT,cont);
    if(argc<4){
        printf("argc is less than 4\n");
    }

    // initialization from ProcessGenerator:
    int Algorithm_type= atoi(argv[1]);   // 1 SFJ 2 HPF 3 RR 4 MultilevelQ
    int quantum=atoi(argv[2]);
    Nprocesses=atoi(argv[3]);

    initClk();

    //Process Table
    ProcessTable = (struct PCB *)malloc(sizeof(struct PCB) * Nprocesses);
    
    for (int i = 0; i < Nprocesses; i++)
    {
        ProcessTable[i] = PCB_def;
    }
    
    #pragma region DS INIT
    if (Algorithm_type == 1) // SJF
    {
        Process_PQueue = CreatePQueue();
        signal(SIGUSR1,finish_SJF);
    }
    else if (Algorithm_type == 2) // HPF
    {
        Process_PQueue = CreatePQueue();
        signal(SIGUSR1,finish_HPF);
    }
    else if (Algorithm_type == 3) // RR
    {
        Process_Queue = CreateQueue();
        signal(SIGUSR1,finish_RR);
    }
    else if (Algorithm_type == 4) // MultilevelQ
    {
        for (int i = 0; i< 12; i++) Process_Queues_Arr[i]=CreateQueue();
        signal(SIGUSR1,finish_MQ);
    }
    #pragma endregion
     
    
    // for Process Generator Communication
    key_t key_id;
    int rec_val, msgq_id;
    key_id = ftok("dummy", 65);
    msgq_id = msgget(key_id, 0666 | IPC_CREAT);

    //for Process.c initial Communication
    char bufferion[20]; //buffer for integer to char conversion
    
    //For Output
   
	logptr = fopen("scheduler.log", "w");
	//perfptr = fopen("scheduler.perf", "w");

    // TODO implement the scheduler :)
    //     1.Start a new process whenever it arrives. (Fork it and give it its parameters)

    // recieve from generator:
    struct PG_msgbuff snt_Process_msg;

    printf("SCHEDULER ID is %d\n",getpid()); 

    while (1 && !finish)
    {
        //recieve message
        rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(struct proc), 2, IPC_NOWAIT);
        if (rec_val != -1)
        {   
            printf("I recieved proc with id %d\n",snt_Process_msg.Process.id);
            //in case of message recieval
            remaining_time = snt_Process_msg.Process.Runtime;
            //update the PCB with remaining time
            ProcessTable[count].remaining_time = remaining_time;
            ProcessTable[count].id = snt_Process_msg.Process.id;

            // fork the process
            
            pid=fork();
            fflush(stdout);
                        //      make; ./process_generator.out  -sch 3 -q 1 &
                        //    A messing AROUND TEST  make; ./process_generator.out -sch 5 -q 2 &
            waiter=1;
            if (!pid) // the child joins the cult of processes
            {

                printf("A child was forked\n");

                my_itoa(remaining_time, bufferion);
                char *args[] = {"./process.out", bufferion, NULL};

                
                execvp(args[0], args);
            }
            // system("ps");
            
            ProcessTable[count].pid = pid;
            count++;

            // system("ps"); 

            // usleep(1000);
            if(waiter){raise(SIGSTOP);}
        }
        // 2.Switch between two processes according to the scheduling algorithm. (stop
        // the old process and save its state and start/resume another one.)
        if (Algorithm_type == 1) // SJF
        {
              if(rec_val!=-1)
            {
                //Add the process in priority queue
                PQ_node = CreatePNode();
                PQ_node->Arrival_Time = snt_Process_msg.Process.ArrivalTime;
                PQ_node->PID = pid;
                PQ_node->Priority = snt_Process_msg.Process.Priority;
                PQ_node->Runtime = snt_Process_msg.Process.Runtime;
                PQ_node->Q_Priority = PQ_node->Runtime;
                EnPQueue(Process_PQueue, PQ_node);

                continue; //go back to the beginning of the loop to check for any process with the same arrival time
            }
            if(RunningPID == -1) //No process is running
            {
                if(PPeek(Process_PQueue)) //if ready queue is not empty
                {
                    //Dequeue the process
                    PQ_node = DePQueue(Process_PQueue);
                    RunningPID = PQ_node->PID;
                    pcb = findPCB(RunningPID, ProcessTable);
                    pcb->state = 1 ; //Running
                    pcb->waiting_time = getClk() - PQ_node->Arrival_Time;
                    pcb->execution_time = PQ_node->Runtime;

                    //print in output file    
                    logptr = fopen("scheduler.log", "a");
                    fprintf(logptr,"At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, PQ_node->Runtime, PQ_node->Runtime, pcb->waiting_time);
                    fclose(logptr);

                    //update variables
                    Total_Execution_Time += PQ_node->Runtime;
                    Total_Waiting_Time += pcb->waiting_time;
                    PreviousCycle = getClk();
                    ArrivalTime = PQ_node->Arrival_Time;
                    free(PQ_node);
                    //start running
                    kill(RunningPID,SIGCONT);
                }
            }
            else //if a process is running decrement the remaining time if a cycle has passed
            {
                if(getClk() == PreviousCycle+1) //1 cycle passed
                {
                    PreviousCycle = getClk();
                    pcb = findPCB(RunningPID, ProcessTable);
                    pcb->remaining_time -= 1;
                    updateWaiting(ProcessTable);
                }
            }
        }
        else if (Algorithm_type == 2)//HPF
        {
            if (rec_val != -1)
            {
                // intialize the PQ_Node
                //snt_Process_msg.Process.SchPriority = snt_Process_msg.Process.Priority;
                // struct Node*node=malloc(sizeof(struct Node));
                PQ_node = CreatePNode();
                PQ_node->PID = pid;
                PQ_node->Priority = snt_Process_msg.Process.Priority;
                PQ_node->Q_Priority = snt_Process_msg.Process.Priority;
                PQ_node->Arrival_Time = snt_Process_msg.Process.ArrivalTime;
                PQ_node->Runtime = snt_Process_msg.Process.Runtime;
                // struct proc*proc=malloc(sizeof(struct proc));
                // *proc=snt_Process_msg.Process;
                // node->process=proc;
                EnPQueue(Process_PQueue, PQ_node);
                pcb = findPCB(Process_PQueue->Header->PID, ProcessTable); //this will always contain the head with higher priority
            }
                //printf("%d\n",pcb->state);
                if (pcb&&!pcb->state)
                {
                    PreviousCycle=getClk();
                    //law mafee4 next ebda2 3la tool 
                    if (!(Process_PQueue->Header->Next))
                    {
                        printf("Starting!\n");
                        PQ_node=PPeek(Process_PQueue);
                        //pcb->waiting_time = (getClk()) - (PQ_node->Arrival_Time) - (pcb->execution_time);
                        pcb->state = 1;
                        kill(pcb->pid, SIGCONT);
                        Start_Time = getClk();
                        if(pcb->execution_time==0)
                        {
                            fprintf(logptr,"At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time+pcb->remaining_time,pcb->remaining_time , pcb->waiting_time);
                            //printf("At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time+pcb->remaining_time,pcb->remaining_time , pcb->waiting_time);
                        }
                        else
                        {
                            fprintf(logptr,"At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time+pcb->remaining_time,pcb->remaining_time , pcb->waiting_time);
                            //printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time+pcb->remaining_time,pcb->remaining_time , pcb->waiting_time);
                        }
                    }
                    else
                    {
                        //something else should be running then.
                        printf("Context Switch\n");
                        pid = Process_PQueue->Header->Next->PID;
                        temp = findPCB(pid, ProcessTable);
                        if(temp->state==1)
                        {
                            temp->state = 0;
                            kill(temp->pid, SIGSTOP);
                            // temp->execution_time+=getClk()-Start_Time;
                            // temp->remaining_time-=getClk()-Start_Time;
                            //by here we should have the next process in PQ_Node
                            PQ_node=Process_PQueue->Header->Next;
                            fprintf(logptr,"At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), temp->id, PQ_node->Arrival_Time, temp->execution_time+temp->remaining_time,temp->remaining_time , temp->waiting_time);
                            printf("At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), temp->id, PQ_node->Arrival_Time, temp->execution_time+temp->remaining_time,temp->remaining_time , temp->waiting_time);
                        }
                        PQ_node=PPeek(Process_PQueue);
                        // pcb->waiting_time=(getClk())-(PQ_node->Arrival_Time)-(pcb->execution_time)+(pcb->remaining_time);
                        pcb->state = 1;
                        kill(pcb->pid, SIGCONT);
                        Start_Time=getClk();
                        if(pcb->execution_time==0)
                        {
                            fprintf(logptr,"At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time+pcb->remaining_time,pcb->remaining_time , pcb->waiting_time);
                            //printf("At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time+pcb->remaining_time,pcb->remaining_time , pcb->waiting_time);
                        }
                        else
                        {
                            fprintf(logptr,"At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time+pcb->remaining_time,pcb->remaining_time , pcb->waiting_time);
                            //printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time+pcb->remaining_time,pcb->remaining_time , pcb->waiting_time);
                        }
                    }
                }

            if(getClk()== PreviousCycle+1){
                printf("a cycle passed!\n");
                PreviousCycle=getClk();
                if(PQ_node!=NULL){

                    pcb->remaining_time -= 1;
                    pcb->execution_time += 1;
                    Total_Execution_Time += 1;
                    updateWaiting(ProcessTable);
                    if(pcb->id != -1) printf("Remaining time in scheduler is %d\n",pcb->remaining_time);
                }
                if (PQ_node != NULL && pcb->remaining_time == 0)
                {
                    printf("Finished!!!\n");
                    PQ_node=DePQueue(Process_PQueue);
                    temp= findPCB(PQ_node->PID, ProcessTable);
                    //temp->execution_time += getClk() - Start_Time;
                    //temp->remaining_time -= getClk() - Start_Time;
                    fprintf(logptr, "At time %d process %d finished arr %d total %d remain 0 wait %d TA %d WTA %.2f\n", getClk(), temp->id, PQ_node->Arrival_Time, temp->execution_time + temp->remaining_time, temp->waiting_time,getClk() - PQ_node->Arrival_Time,(float)(getClk() - PQ_node->Arrival_Time) / temp->execution_time);
                    Total_WTA += (float)(getClk() - PQ_node->Arrival_Time) / temp->execution_time;
                    Total_Waiting_Time+=temp->waiting_time;
                    FinishedProc+=1;
                    free(PQ_node);
                    PQ_node=NULL;
                    temp->execution_time = 0;
                    temp->remaining_time = 0;
                    temp->waiting_time = 0;
                    temp->id = -1;
                    temp->pid = -1;
                    if (Process_PQueue->Header)
                        pcb = findPCB(Process_PQueue->Header->PID, ProcessTable);
                    // {
                    //     pcb->state = 1;
                    //     PQ_node=PPeek(Process_PQueue);
                    //     //pcb->waiting_time = (getClk()) - (PQ_node->Arrival_Time) - (pcb->execution_time);
                    //     kill(PQ_node->PID, SIGCONT);
                    //     Start_Time = getClk();
                    //     if (pcb->execution_time == 0)
                    //     {
                    //         fprintf(logptr, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time + pcb->remaining_time, pcb->remaining_time, pcb->waiting_time);
                    //     }
                    //     else
                    //     {
                    //         fprintf(logptr, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time + pcb->remaining_time, pcb->remaining_time,pcb->waiting_time);
                    //     }
                    // }
                }

                if (isPCBempty(ProcessTable) && FinishedProc == Nprocesses)
                {
                    goto barra;
                }
            }
        
        }
        else if (Algorithm_type == 3) // RR
        {
            // if recieved a message, create a new node for process

            // printf("\nmy pid is %d\n",getpid());
            // printf("LOLER\n");

            // you should move this up later
            if (rec_val != -1)
            {
                // printf("I recieved and creating\n");
                node = CreateNode();
                node->PID = pid;
                node->Priority = snt_Process_msg.Process.Priority;
                node->Arrival_Time = snt_Process_msg.Process.ArrivalTime;
                node->Runtime = snt_Process_msg.Process.Runtime;
                Enqueue(Process_Queue, node);
                node = NULL;
            }

            if (node == NULL)
            {
                node = Peek(Process_Queue);
            }

            if (node != NULL)
            {
                pid = node->PID;
                pcb = findPCB(pid, ProcessTable);
                // printf("PCB value right now is id %d pid %d execution time %d\n", pcb->id,pcb->pid,pcb->execution_time);
                // how to get waiting????

                // fl bdaya 5ales
                if (pcb->state == 0)
                {
                    pcb->waiting_time = (getClk() - node->Arrival_Time) - pcb->execution_time;
                    // Total_Waiting_Time+=pcb->waiting_time;
                    Start_Time = getClk();
                    PreviousCycle = getClk();
                    if (pcb->execution_time == 0)
                    {

                        fprintf(logptr, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                        printf("At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                    }
                    else if (((node->Runtime) - (pcb->execution_time)) != 0)
                    {
                        fprintf(logptr, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                        printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                    }
                }
                kill(pid, SIGCONT); // SIGCONT=18
                pcb->state = 1;
            }

            // law 3adda el wa2t

            if (getClk() == PreviousCycle + 1)
            {
                printf("a cycle passed!\n");
                PreviousCycle = getClk();
                if (node != NULL)
                {
                    pcb->remaining_time -= 1;
                    pcb->execution_time += 1;
                    Total_Execution_Time += 1;
                    // updateWaiting(ProcessTable);
                }

                // if quantum is finished
                if (node != NULL && ((getClk() - Start_Time) == quantum || pcb->remaining_time == 0))
                {
                    node = Dequeue(Process_Queue);
                    // CALCULATE THINGS in PCB
                    // pcb->remaining_time -= quantum;
                    // pcb->execution_time += quantum;
                    // Total_Execution_Time += quantum;
                    pcb->state = 0;
                    if (pcb->remaining_time != 0)
                    {
                        kill(pid, SIGSTOP); // SIGSTOP=19
                        fprintf(logptr, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                        
                        //check for recieved messages first
                        rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(struct proc), 2, IPC_NOWAIT);
                        while (rec_val != -1)
                        {
                            printf("I recieved proc with id %d\n", snt_Process_msg.Process.id);
                            // in case of message recieval
                            remaining_time = snt_Process_msg.Process.Runtime;
                            // update the PCB with remaining time
                            ProcessTable[count].remaining_time = remaining_time;
                            ProcessTable[count].id = snt_Process_msg.Process.id;

                            // fork the process

                            pid = fork();
                            fflush(stdout);
                            //      make; ./process_generator.out  -sch 3 -q 1 &
                            //    A messing AROUND TEST  make; ./process_generator.out -sch 5 -q 2 &
                            waiter = 1;
                            if (!pid) // the child joins the cult of processes
                            {

                                printf("A child was forked\n");

                                my_itoa(remaining_time, bufferion);
                                char *args[] = {"./process.out", bufferion, NULL};

                                execvp(args[0], args);
                            }
                            // system("ps");

                            ProcessTable[count].pid = pid;
                            count++;

                            // system("ps");

                            // usleep(1000);
                            if (waiter)
                            {
                                raise(SIGSTOP);
                            }
                            tempnode = CreateNode();
                            tempnode->PID = pid;
                            tempnode->Priority = snt_Process_msg.Process.Priority;
                            tempnode->Arrival_Time = snt_Process_msg.Process.ArrivalTime;
                            tempnode->Runtime = snt_Process_msg.Process.Runtime;
                            Enqueue(Process_Queue, tempnode);
                            tempnode = NULL;
                            

                            rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(struct proc), 2, IPC_NOWAIT);
                        }

                        Enqueue(Process_Queue, node);
                    }
                    else
                    {
                        // Output & Delete
                        pcb->execution_time = getClk() - node->Arrival_Time - pcb->waiting_time;
                        pcb->remaining_time = node->Runtime - pcb->execution_time;
                        fprintf(logptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, 0, pcb->waiting_time, getClk() - node->Arrival_Time, (float)(getClk() - node->Arrival_Time) / pcb->execution_time);
                        printf("At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, 0, pcb->waiting_time, getClk() - node->Arrival_Time, (float)(getClk() - node->Arrival_Time) / pcb->execution_time);
                        // system("ps");
                        Total_WTA += (float)(getClk() - node->Arrival_Time) / pcb->execution_time;
                        Total_Waiting_Time += pcb->waiting_time;
                        // Delete data
                        pcb->execution_time = 0;
                        pcb->remaining_time = 0;
                        pcb->waiting_time = 0;
                        pcb->id = -1;
                        pcb->pid = -1;
                        free(node);
                        FinishedProc++;
                        // system("ps");
                        // sleep(1);
                        if (isPCBempty(ProcessTable) && FinishedProc == Nprocesses)
                        {
                            finish = 1;
                        }
                    }
                    node = NULL;

                    // FINISH CODEEEE
                    //  kill(pid, SIGSTOP); // SIGSTOP=19

                    // system("ps");
                }
            }
        }
        else if (Algorithm_type == 4)//MultilevelQ
        {
            if (rec_val != -1)
            {
                if (snt_Process_msg.Process.Runtime){
                    struct Node* nodeToBeAdded = CreateNode();
                    nodeToBeAdded->PID = pid;
                    nodeToBeAdded->Priority = snt_Process_msg.Process.Priority;
                    nodeToBeAdded->Priority = (nodeToBeAdded->Priority > 10) ? 10 : nodeToBeAdded->Priority;
                    nodeToBeAdded->Priority = (nodeToBeAdded->Priority < 0) ? 0 : nodeToBeAdded->Priority;
                    nodeToBeAdded->Arrival_Time = snt_Process_msg.Process.ArrivalTime;
                    nodeToBeAdded->Runtime = snt_Process_msg.Process.Runtime;
                    Enqueue(Process_Queues_Arr[nodeToBeAdded->Priority], nodeToBeAdded);
                }
                else {
                    FinishedProc++;
                }
            }
            if (CurrentPriority == -1) {
                for (int i = 0; i <= 10; i++) {
                    node = Dequeue(Process_Queues_Arr[i]);
                    if (node) {
                        CurrentPriority = i;
                        break;
                    }
                }
                if (!node){
                    struct Node* nodeToBeScattered = Dequeue(Process_Queues_Arr[11]);
                    while (nodeToBeScattered){
                        Enqueue(Process_Queues_Arr[nodeToBeScattered->Priority],nodeToBeScattered);
                        nodeToBeScattered = Dequeue(Process_Queues_Arr[11]);
                    }
                }
            }
            if (node && !pcb)
            {
                pid = node->PID;
                pcb = findPCB(pid, ProcessTable);
                if (pcb->state == 0)
                {
                    pcb->waiting_time = (getClk() - node->Arrival_Time) - pcb->execution_time;
                    Start_Time = getClk();
                    PreviousCycle = getClk();
                    if (pcb->execution_time == 0)
                        fprintf(logptr, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                    else if (((node->Runtime) - (pcb->execution_time)) != 0)
                        fprintf(logptr, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);
                    kill(pid, SIGCONT);
                    printf("process %d started its quantum remain %d\n", pcb->id, (node->Runtime) - (pcb->execution_time));
                    pcb->state = 1;
                }
            }

            if (getClk() == PreviousCycle + 1)
            {
                PreviousCycle = getClk();
                if (node)
                {
                    pcb->remaining_time -= 1;
                    pcb->execution_time += 1;
                    Total_Execution_Time += 1;
                    // if quantum is finished
                    if ((getClk() - Start_Time) == quantum || pcb->remaining_time == 0)
                    {
                        pcb->state = 0;
                        if (pcb->remaining_time != 0)
                        {
                            printf("At time %d process %d finished its quantum, remaining time %d\n", getClk(), pcb->id, (node->Runtime) - (pcb->execution_time));
                            kill(pid, SIGSTOP); // SIGSTOP=19
                            fprintf(logptr, "At time %d process %d stopped arr %d total %d remain %d wait %d\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, (node->Runtime) - (pcb->execution_time), pcb->waiting_time);

                            // check for recieved messages first
                            //Hello Nashar UNCOMMENT ME  if you think I am right ####
                            // rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(struct proc), 2, IPC_NOWAIT);
                            // while (rec_val != -1)
                            // {
                            //     printf("I recieved proc with id %d\n", snt_Process_msg.Process.id);
                            //     // in case of message recieval
                            //     remaining_time = snt_Process_msg.Process.Runtime;
                            //     // update the PCB with remaining time
                            //     ProcessTable[count].remaining_time = remaining_time;
                            //     ProcessTable[count].id = snt_Process_msg.Process.id;

                            //     // fork the process

                            //     pid = fork();
                            //     fflush(stdout);
                            //     //      make; ./process_generator.out  -sch 3 -q 1 &
                            //     //    A messing AROUND TEST  make; ./process_generator.out -sch 5 -q 2 &
                            //     waiter = 1;
                            //     if (!pid) // the child joins the cult of processes
                            //     {

                            //         printf("A child was forked\n");

                            //         my_itoa(remaining_time, bufferion);
                            //         char *args[] = {"./process.out", bufferion, NULL};

                            //         execvp(args[0], args);
                            //     }
                            //     // system("ps");

                            //     ProcessTable[count].pid = pid;
                            //     count++;

                            //     // system("ps");

                            //     // usleep(1000);
                            //     if (waiter)
                            //     {
                            //         raise(SIGSTOP);
                            //     }

                            //     struct Node *nodeToBeAdded = CreateNode();
                            //     nodeToBeAdded->PID = pid;
                            //     nodeToBeAdded->Priority = snt_Process_msg.Process.Priority;
                            //     nodeToBeAdded->Priority = (nodeToBeAdded->Priority > 10) ? 10 : nodeToBeAdded->Priority;
                            //     nodeToBeAdded->Priority = (nodeToBeAdded->Priority < 0) ? 0 : nodeToBeAdded->Priority;
                            //     nodeToBeAdded->Arrival_Time = snt_Process_msg.Process.ArrivalTime;
                            //     nodeToBeAdded->Runtime = snt_Process_msg.Process.Runtime;
                            //     Enqueue(Process_Queues_Arr[nodeToBeAdded->Priority], nodeToBeAdded);

                            //     rec_val = msgrcv(msgq_id, &snt_Process_msg, sizeof(struct proc), 2, IPC_NOWAIT);
                            // }

                            //#### End of Uncomment
                            
                            Enqueue(Process_Queues_Arr[CurrentPriority+1], node);
                        }
                        else
                        {
                            // Output & Delete
                            printf("At time %d process %d finished and terminated\n", getClk(), pcb->id);
                            pcb->execution_time = getClk() - node->Arrival_Time - pcb->waiting_time;
                            pcb->remaining_time = node->Runtime - pcb->execution_time;
                            fprintf(logptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, node->Arrival_Time, node->Runtime, 0, pcb->waiting_time, getClk() - node->Arrival_Time, (float)(getClk() - node->Arrival_Time) / pcb->execution_time);
                            Total_WTA += (float)(getClk() - node->Arrival_Time) / pcb->execution_time;
                            Total_Waiting_Time += pcb->waiting_time;
                            // Delete data
                            pcb->execution_time = 0;
                            pcb->remaining_time = 0;
                            pcb->waiting_time = 0;
                            pcb->id = -1;
                            pcb->pid = -1;
                            free(node);
                            FinishedProc++;
                            if (isPCBempty(ProcessTable) && FinishedProc == Nprocesses)
                            {
                                finish = 1;
                            }
                        }
                        node = NULL;
                        pcb = NULL;
                        CurrentPriority = -1;
                    }
                }
            }
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
    printf("I wenta barraaa\n");
    fprintf(perfptr,"CPU Utilization = %.2f%%\n",((float)Total_Execution_Time*100/getClk()));    
    fprintf(perfptr,"Avg WTA=%.2f\n",(Total_WTA/Nprocesses));    
    fprintf(perfptr,"Avg Waiting = %.2f\n",((float)Total_Waiting_Time/Nprocesses));    

    //fclose(logptr);
    fclose(perfptr);
    
    
    
    free(ProcessTable);
    if (Algorithm_type == 1) // SJF
    {
        free(Process_PQueue);
    }
    else if (Algorithm_type == 2) // HPF
    {
        fclose(logptr);
        free(Process_PQueue);
    }
    else if (Algorithm_type == 3) // RR
    {
        fclose(logptr);
        free(Process_Queue);
    }
    else if (Algorithm_type == 4) // MultilevelQ
    {
        fclose(logptr);
        for (int i = 0; i < 12; i++) free(Process_Queues_Arr[i]);
    }
    // upon termination release the clock resources.
    destroyClk(true);


}


void finish_SJF()
{
    //process is terminated
    RunningPID = -1;
    //calculate TA, WTA
    int TA = getClk() - ArrivalTime;
    float WTA = (TA * 1.0) / (pcb->execution_time);

    //print in output file
    logptr = fopen("scheduler.log", "a");
    fprintf(logptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), pcb->id, ArrivalTime, pcb->execution_time, 0, pcb->waiting_time, TA, WTA);
    fclose(logptr);
    
    //delet entry in pcb
    Total_WTA += WTA;
    pcb->execution_time = 0;
    pcb->remaining_time = 0;
    pcb->waiting_time = 0;
    pcb->id = -1;
    pcb->pid = -1;
    FinishedProc += 1;
    
    //check if it is the last process
    if (FinishedProc == Nprocesses)
    {
       finish=1;
    }
}  


void finish_HPF()
{
    // printf("Finished!!!\n");
    // PQ_node = DePQueue(Process_PQueue);
    // temp = findPCB(PQ_node->PID, ProcessTable);
    // temp->execution_time = getClk() - PQ_node->Arrival_Time-temp->waiting_time;
    // temp->remaining_time = PQ_node->Runtime-temp->execution_time;
    // fprintf(logptr, "At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n", getClk(), temp->id, PQ_node->Arrival_Time, temp->execution_time + temp->remaining_time,temp->remaining_time, temp->waiting_time, getClk() - PQ_node->Arrival_Time, (float)(getClk() - PQ_node->Arrival_Time) / temp->execution_time);
    // Total_WTA += (float)(getClk() - PQ_node->Arrival_Time) / temp->execution_time;
    // Total_Waiting_Time += temp->waiting_time;
    // FinishedProc += 1;
    // free(PQ_node);
    // temp->execution_time = 0;
    // temp->remaining_time = 0;
    // temp->waiting_time = 0;
    // temp->id = -1;
    // temp->pid = -1;
    // if (Process_PQueue->Header)
    // {
    //     pcb = findPCB(Process_PQueue->Header->PID, ProcessTable);
    //     pcb->state = 1;
    //     PQ_node = PPeek(Process_PQueue);
    //     pcb->waiting_time = (getClk()) - (PQ_node->Arrival_Time) - (pcb->execution_time);
    //     kill(PQ_node->PID, SIGCONT);
    //     Start_Time = getClk();
    //     if (pcb->execution_time == 0)
    //     {
    //         fprintf(logptr, "At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time + pcb->remaining_time, pcb->remaining_time, pcb->waiting_time);
    //         //printf("At time %d process %d started arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time + pcb->remaining_time, pcb->remaining_time, pcb->waiting_time);
    //     }
    //     else
    //     {
    //         fprintf(logptr, "At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time + pcb->remaining_time, pcb->remaining_time, pcb->waiting_time);
    //         //printf("At time %d process %d resumed arr %d total %d remain %d wait %d\n", getClk(), pcb->id, PQ_node->Arrival_Time, pcb->execution_time + pcb->remaining_time, pcb->remaining_time, pcb->waiting_time);
    //     }
    // }

    // if (isPCBempty(ProcessTable) && FinishedProc == Nprocesses)
    // {
    //     finish=1;
    // }
}  

void finish_RR()
{
    ;
}

void finish_MQ()
{
    ;
}  