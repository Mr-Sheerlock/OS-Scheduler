#include"headers.h"

/* Modify this file as needed*/
int remainingtime;



int main(int agrc, char * argv[])
{
    initClk();
    
    //printf("\n 3) Process Starting!\n");
    // system("ps &");

    raise(SIGSTOP);
    //TODO it needs to get the remaining time from somewhere
    // printf(" process started\n");

    remainingtime = atoi(argv[1]);
    printf("clk in process = %d\n", getClk());
    // if(remainingtime == 0)
    // {
    //     raise(SIGSTOP);
    // }
    while (remainingtime > 0)
    {
        // if(!running)
        // {
        //     raise(SIGSTOP);
        // }
        //else
        //{
            int x = getClk();
            while(x==getClk()); //passes 1 second

            remainingtime--;
            printf("remaining time is %d\n ",remainingtime);
        //}
    }
    //printf("12) Process Finished!\n");
    destroyClk(false);
    
   return 0;
}


    
 
