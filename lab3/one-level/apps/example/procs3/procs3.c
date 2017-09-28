#include "usertraps.h"
#include "misc.h"

#define N 3500

void callStack (void) {
  int arr[N];
  int i;
  for (i=0; i<N; i++) {
  	arr[i] = i;
  }
  Printf("Successfully initalized the array of %d INTs\n",N);
  Printf("There should be %d Page allocations\n",N/1024);
  Printf("1st elem: %d, Last elem: %d\n", arr[0], arr[N-1]);
}



void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);


  // Now print a message to show that everything worked
  Printf("Process (%d): Trying to cause the stack to grow!\n", getpid());

  callStack();

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

    Exit();
}
