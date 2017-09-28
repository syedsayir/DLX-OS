#include "lab2-api.h"
#include "usertraps.h"
#include "misc.h"
#include "spawn.h"

atmosphere *atm;            // Used to access missile codes in shared memory page
void oReady() {
	atm->o_atoms++;
	Printf("%d O\n", atm->o_atoms);
}
void makeOxygen() {
	atm->o_atoms -= 2;
	atm->o2_molecules++;
	Printf("O + O -> O2\n");
}
void makeOzone() {
	atm->o2_molecules -= 3;
	atm->o3_molecules += 2;
	Printf("3O2 -> 2O3\n");
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
	oReady();
	if (((atm->n2_molecules)>=1) && ((atm->o2_molecules)>=2)) {
		if (cond_signal(c_wake_n) != 0) {
		Printf("Bad Condition Variable c_wake_n (%d) in ", c_wake_n); Printf(argv[0]); Printf("\n");
		Exit();
	}
		if (lock_release(s_lock) != SYNC_SUCCESS) {
			Printf("Bad Lock s_lock (%d) in ", s_lock); Printf(argv[0]); Printf("\n");
			Exit();
		}
		if (lock_acquire(s_lock) != SYNC_SUCCESS) {
			Printf("Bad Lock s_lock (%d) in ", s_lock); Printf(argv[0]); Printf("\n");
			Exit();
		}
	}
	if ((atm->o_atoms)>=2) {
		makeOxygen();
		if (((atm->n2_molecules)>=1) && ((atm->o2_molecules)>=2)) {
			cond_signal(c_wake_n);
		}
	}
	if (lock_release(s_lock) != SYNC_SUCCESS) {
		Printf("Bad Lock s_lock (%d) in ", s_lock); Printf(argv[0]); Printf("\n");
		Exit();
	}
	if (lock_acquire(s_lock) != SYNC_SUCCESS) {
		Printf("Bad Lock s_lock (%d) in ", s_lock); Printf(argv[0]); Printf("\n");
		Exit();
	}

	if (((atm->o2_molecules)>=3) && temp >= 50) {
		makeOzone();
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
