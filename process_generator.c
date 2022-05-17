#include "headers.h"

int msgid;

struct proc{
    int pid;
    int id;
    int ArrivalTime;
    int RunTime;
    int Priority;
    int SchPriority;  // To Be Assigned in Scheduler Based on Algorithm
    int MemS; //memory size
};
struct msg{
    long mtype;
    struct proc Process;
};
struct LNode{
    struct proc *process;
    struct LNode*Next;
}Node_def={NULL,NULL};
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
    if(node->process->SchPriority<=list->Head->process->SchPriority)
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
        if(node->process->SchPriority<=tempptr->Next->process->SchPriority)
        {
            node->Next=tempptr->Next;
            tempptr->Next=node;
            list->count++;
            return;
        }
    } while (tempptr=tempptr->Next);
    
}

//new
int ReadInputFile(struct List * Queue,char*filename)
{
    FILE*InputFile;
    char alpha[1];
    InputFile=fopen(filename,"r");
    while(1)
    {
        struct LNode*node=calloc(1,sizeof(struct LNode));
        struct proc*process=calloc(1,sizeof(struct proc));
        long x=ftell(InputFile);
        int y=fscanf(InputFile,"%c",alpha);
        if(y==-1){fclose(InputFile);return 1;}
        if(*alpha=='#'){fscanf(InputFile,"%*[^\n]\n");continue;}
        fseek(InputFile,x,SEEK_SET);
        y=fscanf(InputFile,"%d",&process->id);
        if(!y){return 0;}
        y=fscanf(InputFile,"%d",&process->ArrivalTime);
        if(!y){return 0;}
        y=fscanf(InputFile,"%d",&process->RunTime);
        if(!y){return 0;}
        y=fscanf(InputFile,"%d",&process->Priority);        
        if(!y){return 0;}

        y=fscanf(InputFile,"%d%*[^\n]\n",&process->MemS); 
        if(!y){return 0;}


        process->SchPriority=process->ArrivalTime;
        node->process=process;
        if (process->RunTime)
        {
            enqueue(Queue,node);
        }
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
    char* filename=arg[1];
    int summ=0;
    int count=0;
    int Last_Arrival=0;
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
    if(!algo){printf("No Algorithm Specified\n");return 0;}

    //? 1. Read the input files.
    //? 5. Create a data structure for processes and provide it with its parameters.
    //printf("%s\n",arg[0]);
    struct List ProcessQueue=List_def;
    if(!ReadInputFile(&ProcessQueue,filename)){printf("Invalid Input\n");return 0;}  //Read Processes from file and Store in Queue

    //? 3. Initiate and create the scheduler and clock processes.

    system("./clk.out & ");
    char cmd[28];
    // gdb --args ./Process_generator.out -g -sch 3 -q
    sprintf(cmd, "./scheduler.out %s %s %d &", algo, quantum, ProcessQueue.count);
    system(cmd);

    count =ProcessQueue.count;
    //? 4. Use this function after creating the clock process to initialize clock.
    initClk();   // Attach to Clock


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
            //printf("sent\n");
            sentmsg.Process=*(dequeue(&ProcessQueue)->process);
            msgsnd(msgid,&sentmsg,sizeof(struct proc),0);  // Send Process To Scheduler When It Arrives 
            summ+= sentmsg.Process.RunTime;
            Last_Arrival=sentmsg.Process.ArrivalTime;
        }
    }
    //+2 for the case they all arive the same time, to leave time for the output -RR (switching takes time)
    //Last_Arrival solves the case when the sum of Runtimes is lesser than a big gap inbetween
    sleep(summ+count+Last_Arrival); 
    printf("Sleep Finished\n");
    system("ps");
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    int x=msgctl(msgid,IPC_RMID,0);  // Destroy Message Queue
    //TODO Clears all resources in case of interruption
}