#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"

#include "spawn.h"

void main (int argc, char *argv[])
{
	cir_buf *buf;            // Used to access missile codes in shared memory page
	uint32 h_mem;            // Handle to the shared memory page
	sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	sem_t s_emptySlots, s_filledSlots; // Semaphore to signal the process about slots in buffer
	lock_t s_lock; 	         // Semaphore to signal the original process that we're done
	char c;
	int i;
	char* h_buf;
	if (argc != 7) { 
		Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
		Exit();
	} 

	// Convert the command-line strings into integers for use as handles
	h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
	s_procs_completed = dstrtol(argv[2], NULL, 10);
	s_lock = dstrtol(argv[3], NULL, 10);
	h_buf = argv[4];
	s_emptySlots = dstrtol(argv[5], NULL, 10);
	s_filledSlots = dstrtol(argv[6], NULL, 10);

	// Map shared memory page into this process's memory space
	if ((buf = (cir_buf *)shmat(h_mem)) == NULL) {
		Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
		Exit();
	}

	// Now print a message to show that everything worked
	for(i=0;i<stringLen(h_buf);i++)
	{
		if (sem_wait(s_filledSlots) != SYNC_SUCCESS) {
			Printf("Bad semaphore s_filledSlots (%d) in ", s_filledSlots); Printf(argv[0]); Printf("\n");
			Exit();
		}
		if (lock_acquire(s_lock) != SYNC_SUCCESS) {
			Printf("Bad Lock s_lock (%d) in ", s_lock); Printf(argv[0]); Printf("\n");
			Exit();
		}
		c = buf->buffer[buf->tail];
		incTail(buf);
		if (lock_release(s_lock) != SYNC_SUCCESS) {
			Printf("Bad Lock s_lock (%d) in ", s_lock); Printf(argv[0]); Printf("\n");
			Exit();
		}
		Printf("Consumer %d removed: %c\n",Getpid(), c);
		if(sem_signal(s_emptySlots) != SYNC_SUCCESS) {
			Printf("Bad semaphore s_emptySlots (%d) in ", s_emptySlots); Printf(argv[0]); Printf(", exiting...\n");
			Exit();
		}
	}
	// Signal the semaphore to tell the original process that we're done
	Printf("Consumer: PID %d is complete.\n", Getpid());
	if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
		Exit();
	}
}
