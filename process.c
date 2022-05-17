#include"headers.h"

/* Modify this file as needed*/
int remainingtime;

int running=0;
void conthand()
{
    printf("sigcont sent\n");
    running=1;
}

int main(int agrc, char * argv[])
{
    signal(SIGCONT,conthand);
    kill(getppid(),SIGCONT);
    initClk();
    
    
    //TODO it needs to get the remaining time from somewhere

    remainingtime = atoi(argv[1]);
    printf("clk in process = %d\n", getClk());
    if(remainingtime == 0)
    {
        raise(SIGSTOP);
    }
    while (remainingtime > 0)
    {
        if(!running)
        {
            printf("Sigstop raised\n");
            raise(SIGSTOP);
        }
        else
        {
            int x = getClk();
            while(x==getClk()); //passes 1 second

            remainingtime--;
            printf("remaining time is %d\n ",remainingtime);
        }
    }
    kill(getppid(),SIGUSR1);
    
    printf("PROCESS DEAD\n");
    destroyClk(false);
    
   return 0;
}


    
 
