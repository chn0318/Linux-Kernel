#include <sched.h>

int main(int argc,char *argv[])
{

      struct sched_param param;
      int maxpri;
      maxpri = sched_get_priority_max(SCHED_FIFO);
      if(maxpri == -1) 
      {
            perror("sched_get_priority_max() failed");
            exit(1);
      }
      param.sched_priority = maxpri;
      if (sched_setscheduler(0, SCHED_FIFO, &param) == -1) 

     {
            perror("sched_setscheduler() failed");
            exit(1);
     }
     while (1)
     {
         /* code */
     }
     
}