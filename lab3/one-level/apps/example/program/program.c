#include "usertraps.h"
#include "misc.h"

#define MAX 500000

void main (int argc, char *argv[])
{
  sem_t s_prog_complete; // Semaphore to signal the original process that we're done

  int i, count=0;

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_prog_complete = dstrtol(argv[1], NULL, 10);

  // Now print a message to show that everything worked
  Printf("Program (%d): Beginning!\n", getpid());

  //Loop
  for (i=0; i<MAX; i++) {
  	count+=1;
  }

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_prog_complete) != SYNC_SUCCESS) {
    Printf("Program (%d): Bad semaphore s_prog_complete (%d)!\n", getpid(), s_prog_complete);
    Exit();
  }

  Printf("Program (%d): Ended!\n", getpid());
    Exit();
}
