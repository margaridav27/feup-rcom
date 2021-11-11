#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include "alarm.h"

int flag=1, try=1;

void handler()                  
{
	printf("alarme # %d\n", try);
	flag=1;
	try++;
}


int timeout(int argc, char **argv)
{

(void) signal(SIGALRM, handler);

while(try < 4){
   if(flag){
      alarm(3);
      flag=0;
   }
}
printf("Vou terminar.\n");

}

