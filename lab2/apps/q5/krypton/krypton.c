#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

void main (int argc, char *argv[])
{
	int i;
	atmosphere *atm;
	uint32 h_mem;                   // Used to hold handle to shared memory page
	sem_t s_procs_completed;        // Semaphore used to wait until all spawned processes have completed
	lock_t s_lock;                  // Semaphore used to wait until all spawned processes have completed
	cond_t c_wake_n, c_wake_o;     // Cond Variable used to store empty and filled slots of buffer
	char s_lock_str[10];            // Used as command-line argument to pass lock handle to new processes
	char h_mem_str[10];             // Used as command-line argument to pass mem_handle to new processes
	char c_wake_o_str[10];          // Used as command-line argument to pass empty slots in buffer to new processes
	char c_wake_n_str[10];          // Used as command-line argument to pass filled stots in buffer to new processes
	char s_procs_completed_str[10]; // Used as command-line argument to pass page_mapped handle to new processes
	char *temp;			// Used to store temperature of the system
	int numNatoms;
	int numOatoms;

	if (argc != 4) {
		Printf("Usage: "); Printf(argv[0]); Printf(" <# N atoms> <# O atoms> <TEMP>\n");
		Exit();
	}

	// Convert string from ascii command line argument to integer number
	numNatoms = dstrtol(argv[1], NULL, 10); // the "10" means base 10
	numOatoms = dstrtol(argv[2], NULL, 10); // the "10" means base 10
	temp = argv[3];      // the "10" means base 10
	Printf("Creating %d N atoms and %d O atoms at temperature: ", numNatoms, numOatoms);
	Printf(temp);
	Printf(" degrees.\n");

	// Allocate space for a shared memory page, which is exactly 64KB
	// Note that it doesn't matter how much memory we actually need: we 
	// always get 64KB
	if ((h_mem = shmget()) == 0) {
		Printf("ERROR: could not allocate shared memory page in "); Printf(argv[0]); Printf(", exiting...\n");
		Exit();
	}

	// Map shared memory page into this process's memory space
	if ((atm = (atmosphere *)shmat(h_mem)) == NULL) {
		Printf("Could not map the shared page to virtual address in "); Printf(argv[0]); Printf(", exiting..\n");
		Exit();
	}

	//Initialize the buffer
	atm->n_atoms = 0;
	atm->o_atoms = 0;
	atm->n2_molecules = 0;
	atm->o2_molecules = 0;
	atm->no2_molecules = 0;
	atm->o3_molecules = 0;


	// Create a lock for synchronization
	if ((s_lock = lock_create()) == SYNC_FAIL) {
		Printf("Bad lock_create in "); Printf(argv[0]); Printf("\n");
		Exit();
	}
	// Also Create 2 condition variables for synchronization
	if ((c_wake_n = cond_create(s_lock)) == SYNC_FAIL) {
		Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
		Exit();
	}
	if ((c_wake_o = cond_create(s_lock)) == SYNC_FAIL) {
		Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
		Exit();
	}


	// Create semaphore to not exit this process until all other processes 
	// have signalled that they are complete.  To do this, we will initialize
	// the semaphore to (-1) * (number of signals), where "number of signals"
	// should be equal to the number of processes we're spawning - 1.  Once 
	// each of the processes has signaled, the semaphore should be back to
	// zero and the final sem_wait below will return.
	if ((s_procs_completed = sem_create(-((numOatoms+numNatoms-1)))) == SYNC_FAIL) {
		Printf("Bad sem_create in "); Printf(argv[0]); Printf("\n");
		Exit();
	}

	// Setup the command-line arguments for the new process.  We're going to
	// pass the handles to the shared memory page and the semaphore as strings
	// on the command line, so we must first convert them from ints to strings.
	ditoa(h_mem, h_mem_str);
	ditoa(s_procs_completed, s_procs_completed_str);
	ditoa(s_lock, s_lock_str);
	ditoa(c_wake_o, c_wake_o_str);
	ditoa(c_wake_n, c_wake_n_str);

	// Now we can create the processes.  Note that you MUST end your call to
	// process_create with a NULL argument so that the operating system
	// knows how many arguments you are sending.
	for(i=0; i<numNatoms; i++) {
		process_create(FILENAME_TO_RUN_N, h_mem_str, s_procs_completed_str, s_lock_str, c_wake_o_str, c_wake_n_str, temp, NULL);
		//Printf("Producer Process No. %d created\n", i);
	}
	for(i=0; i<numOatoms; i++) {
		process_create(FILENAME_TO_RUN_O, h_mem_str, s_procs_completed_str, s_lock_str, c_wake_o_str, c_wake_n_str, temp, NULL);
		//Printf("Consumer Process No. %d created\n", i);
	}

	// And finally, wait until all spawned processes have finished.
	if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf("\n");
		Exit();
	}
	Printf("Exiting main process.\n");
}
