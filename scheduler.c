#include "headers.h"



struct PCB {
    bool state=0; //0 for waiting, 1 for running 
    int remaining_time;
    int waiting_time;
    int execution_time;

}

struct PG_msgbuff
{
    long mtype;
    int id;
    int Arrival;
    int Runtime;
    int Priority;
};

int main(int argc, char * argv[])
{

    initClk();
    //initialization that is expected from ProcessGenerator:
    int Algorithm_type = 3 ;//atoi(argv[0]);   // 1 SFJ 2 HPF 3 RR 4 MultilevelQ
    int Nprocesses= 3;// atoi(argv[1]);   

    int PIDArr[Nprocesses]; //MIGHT BE REPLACED BY A QUEUE
    struct PCB PCBArr[Nprocesses]; //MIGHT BE REPLACED BY A QUEUE

    
    int count;
    int remaining_time;
    //for Process Generator Communication
    key_t key_id;
    int rec_val, msgq_id;

    key_id = ftok("keyfile", 65);               
    msgq_id = msgget(key_id, 0666 | IPC_CREAT); 
    
    //TODO implement the scheduler :)


//     1.Start a new process whenever it arrives. (Fork it and give it its parameters)

    // recieve from generator: 
    struct PG_msgbuff PGMessage;
    while(1)
    {
        
        /* receive all types of messages */
        rec_val = msgrcv(msgq_id, &PGMessage, sizeof(PGMessage)-sizeof(PG_msgbuff.mtype), 0, !IPC_NOWAIT);

        if (rec_val == -1)
            ;  //pass
        else{
            //calculate remaining time for process
            if (Algorithm_type < 3)
            {
                remaining_time = PGMessage.Runtime;
            }
            elif (Algorithm_type==3)
            {
                // remaining_time = PGMessage.Runtime + 7aga;
            }
            else {
                // remaining_time = PGMessage.Runtime+7aga akeed akbar;
            }
            //e3ml PCB
            
            // fork the process
            PIDArr[count]=fork();
            PCBArr[count].remaining_time=remaining_time;
            if(PIDArr[count]==0){
                //calculate remaining time:
                char *args[]={"./process",(string)remaining_time,NULL};
                execvp(args[0],args);
            }
            }
            
        //2.Switch between two processes according to the scheduling algorithm. (stop 
        //the old process and save its state and start/resume another one.)
        //3.Keep a process control block (PCB) for each process in the system. A PCB
        //should keep track of the state of a process; running/waiting, execution time,
        //remaining time, waiting time, etc.

    }

    //msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);









    //upon termination release the clock resources.
    
    destroyClk(true);
}
