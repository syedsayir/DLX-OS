#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

atmosphere *atm;            // Used to access missile codes in shared memory page
void nReady() {
	atm->n_atoms++;
	Printf("%d N\n", atm->n_atoms);
}
void makeNitrogen() {
	atm->n_atoms -= 2;
	atm->n2_molecules++;
	Printf("N + N -> N2\n");
}
void makeNO2() {
	atm->n2_molecules--;
	atm->o2_molecules -= 2;
	atm->no2_molecules += 2;
	Printf("N2 + 2O2 -> 2NO2\n");
}
void main (int argc, char *argv[])
{
	uint32 h_mem;            // Handle to the shared memory page
	sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	lock_t s_lock; 	         // Semaphore to signal the original process that we're done
	int i;
	cond_t c_wake_n, c_wake_o;     // Cond Variable used to store empty and filled slots of buffer
	int temp;
	if (argc != 7) { 
		Printf("Usage: "); Printf(argv[0]); Printf(" <handle_to_shared_memory_page> <handle_to_page_mapped_semaphore>\n"); 
		Exit();
	} 

	// Convert the command-line strings into integers for use as handles
	h_mem = dstrtol(argv[1], NULL, 10); // The "10" means base 10
	s_procs_completed = dstrtol(argv[2], NULL, 10);
	s_lock = dstrtol(argv[3], NULL, 10);
	c_wake_o = dstrtol(argv[4], NULL, 10);
	c_wake_n = dstrtol(argv[5], NULL, 10);
	temp = dstrtol(argv[6], NULL, 10);

	// Map shared memory page into this process's memory space
	if ((atm = (atmosphere *)shmat(h_mem)) == NULL) {
		Printf("Could not map the virtual address to the memory in "); Printf(argv[0]); Printf(", exiting...\n");
		Exit();
	}

	if (lock_acquire(s_lock) != SYNC_SUCCESS) {
		Printf("Bad Lock s_lock (%d) in ", s_lock); Printf(argv[0]); Printf("\n");
		Exit();
	}
	nReady();
	if ((atm->n_atoms)>=2) {
		makeNitrogen();
		if (((atm->n2_molecules)>=1) && ((atm->o2_molecules)>=2) && temp <= 100) {
			makeNO2();
		}
	}
	if (cond_wait(c_wake_n) != 0) {
		Printf("Bad Condition Variable c_wake_n (%d) in ", c_wake_n); Printf(argv[0]); Printf("\n");
		Exit();
	}
	if (((atm->n2_molecules)>=1) && ((atm->o2_molecules)>=2) && temp <= 100) {
		makeNO2();
	}
	if (lock_release(s_lock) != SYNC_SUCCESS) {
		Printf("Bad Lock s_lock (%d) in ", s_lock); Printf(argv[0]); Printf("\n");
		Exit();
	}


	if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in ", s_procs_completed); Printf(argv[0]); Printf(", exiting...\n");
		Exit();
	}
}
