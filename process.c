#include"headers.h"

/* Modify this file as needed*/
int remainingtime;



int main(int agrc, char * argv[])
{
    initClk();
    
    printf("\n Process Starting!\n");
    // system("ps &");

    raise(SIGSTOP);
    //TODO it needs to get the remaining time from somewhere
    printf(" process started");

    remainingtime = atoi(argv[1]);
    while (remainingtime > 0)
    {
        int x = getClk();
        while(x==getClk()); //passes 1 second

        remainingtime--;
        printf("remaining time is %d\n",remainingtime);
    }
    
    destroyClk(false);
    
   return 0;
}


    
 
