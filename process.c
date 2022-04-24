#include "headers.h"

/* Modify this file as needed*/
int remainingtime;



int main(int agrc, char * argv[])
{
    initClk();
    signal(SIGUSR1,handler1);
    signal(SIGUSR2,handler2);

    //TODO it needs to get the remaining time from somewhere
    
  
    remainingtime = argv[1];
    while (remainingtime > 0)
    {
        remainingtime--;
    }
    
   return 0;
}

    void handler1(int signum)
{
 //Stop
 
  signal(SIGUSR1,handler1);
}

void handler2(int signum)
{
 //Continue
 
  signal(SIGUSR2,handler2);
}

    destroyClk(false);
    
 
