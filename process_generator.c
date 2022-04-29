#include "headers.h"
int msgid;

struct proc{
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
struct LNode{
    struct proc *process;
    struct LNode*Next;
}LNode_def={NULL,NULL};
struct List{
    struct LNode*Head;
    int count;
}List_def={NULL,0};
// Priority Queue Enqueue and Dequeue Functions
struct LNode* dequeue(struct List*list)
{
    if(!list->Head){return NULL;}
    struct LNode* temp=list->Head;
    list->Head=list->Head->Next;
    return temp;
}
void enqueue(struct List*list,struct LNode* node)
{
    if(!list->Head){list->Head=node;list->count++;return;}
    if(node->process->ArrivalTime<=list->Head->process->ArrivalTime)
    {
        node->Next=list->Head;
        list->Head=node;
        list->count++;
        return;
    }
    struct LNode * tempptr=list->Head;
    do
    {
        if(!tempptr->Next){tempptr->Next=node;list->count++;return;}
        if(node->process->ArrivalTime<=tempptr->Next->process->ArrivalTime)
        {
            node->Next=tempptr->Next;
            tempptr->Next=node;
            list->count++;
            return;
        }
    } while (tempptr=tempptr->Next);

}
void ReadInputFile(struct List * Queue)
{
    FILE*InputFile;
    int c=0;
    char alpha[1];
    InputFile=fopen("testcase.txt","r");
    while(1)
    {
        struct LNode*node=calloc(1,sizeof(struct LNode));
        struct proc*process=calloc(1,sizeof(struct proc));
        int x=fscanf(InputFile,"%c\n",alpha);
        if(x==-1||x==0){fclose(InputFile);break;}
        if(*alpha=='#'){fscanf(InputFile,"%*[^\n]\n");continue;}
        process->id=atoi(alpha);
        fscanf(InputFile,"%d\n",&process->ArrivalTime);
        fscanf(InputFile,"%d\n",&process->RunTime);
        fscanf(InputFile,"%d\n",&process->Priority);
        c++;
        node->process=process;
        enqueue(Queue,node);
    }

}

void clearResources(int);

int main(int argc, char *arg[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    //? 2. Read the chosen scheduling algorithm and its parameters, if there are any from the argument list.

    int opt=0;
    char* quantum="1";
    char* algo=NULL;
 
    while((opt=getopt(argc,arg,"q:sch:"))!=-1) //Command Line Option Parsing
    {
        if(opt=='q')
        {
            quantum=optarg;
        }
        if(opt=='h')
        {
            algo=optarg;
        }
    }
    if(!algo){printf("No Algorithm Specified");return 0;}

    //? 1. Read the input files.
    //? 5. Create a data structure for processes and provide it with its parameters.

    struct List ProcessQueue=List_def;
    ReadInputFile(&ProcessQueue);  //Read Processes from file and Store in Queue
    char procnum[4];
    snprintf(procnum,4,"%d",ProcessQueue.count);

    //? 3. Initiate and create the scheduler and clock processes.

    //      ./process_generator.out testcase.txt -sch 3 -q 1 &
    //      ./process_generator.out testcase.txt -sch 5 -q 2 &
    //doesn't reach here
    int clkpid=fork();
    if(!clkpid)
    {
        execl("./clk.out","/clk.out",(char*)NULL);  //Start Clock
    }
    int schedulerpid=fork();
    if(!schedulerpid)
        execl("./scheduler.out","/scheduler.out",algo,quantum,procnum,(char*)NULL);  //Start Scheduler 


    //? 4. Use this function after creating the clock process to initialize clock.
    initClk();   // Attach to Clock
    // int x = getClk();
    // printf("Current Time is %d\n", x);

    //? 6. Send the information to the scheduler at the appropriate time.
    // TODO Generation Main Loop
    struct msg sentmsg;
    sentmsg.mtype=2;
    key_t key=ftok("dummy",65);        // Create Messsage Queue
    msgid=msgget(key,IPC_CREAT|0666);
    while(1)
    {
        if(!ProcessQueue.Head){break;}
        if(ProcessQueue.Head->process->ArrivalTime==getClk())
        {
            sentmsg.Process=*(dequeue(&ProcessQueue)->process);
            msgsnd(msgid,&sentmsg,sizeof(struct proc),0);  // Send Process To Scheduler When It Arrives 
            //printf("Sent %d\n",dequeue(&ProcessQueue)->process->pid);
        }
    }
    //should send signal to scheduler that it finished.
    kill(schedulerpid,SIGUSR2);
    sleep(15);
    //kill(schedulerpid,SIGKILL);
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    int x=msgctl(msgid,IPC_RMID,0);  // Destroy Message Queue
    //TODO Clears all resources in case of interruption
}