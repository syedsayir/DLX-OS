#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define PROCS1 "procs1.dlx.obj"
#define PROCS2 "procs2.dlx.obj"
#define PROCS3 "procs3.dlx.obj"
#define PROG "program.dlx.obj"
#define NUM 80960
#define INST 30

int x(int u){

	//*p = 'a';
	return 100;
}
void main (int argc, char *argv[])
{
	int num_hello_world = 0;             // Used to store number of processes to create
	int i;                               // Loop index variable
	sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
	char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes
	sem_t s_prog_complete;             // Semaphore used to wait until all spawned processes have completed
	char s_prog_complete_str[10];      // Used as command-line argument to pass page_mapped handle to new processes


	if (argc != 2) {
		Printf("Usage: %s <number of hello world processes to create>\n", argv[0]);
		Exit();
	}

	// Convert string from ascii command line argument to integer number
	num_hello_world = dstrtol(argv[1], NULL, 10); // the "10" means base 10
	//Printf("makeprocs (%d): Creating %d hello_world processes\n", getpid(), num_hello_world);

	// Create semaphore to not exit this process until all other processes 
	// have signalled that they are complete.
	if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
		Printf("makeprocs (%d): Bad sem_create\n", getpid());
		Exit();
	}
	if ((s_prog_complete = sem_create(0)) == SYNC_FAIL) {
		Printf("makeprocs (%d): Bad sem_create\n", getpid());
		Exit();
	}

	// Setup the command-line arguments for the new processes.  We're going to
	// pass the handles to the semaphore as strings
	// on the command line, so we must first convert them from ints to strings.
	ditoa(s_procs_completed, s_procs_completed_str);
	ditoa(s_prog_complete, s_prog_complete_str);

	// Create Hello World process
	Printf("-------------------------------------------------------------------------------------\n");
	Printf("makeprocs (%d): Creating hello world\n", getpid());
	process_create(HELLO_WORLD, s_procs_completed_str, NULL);
	if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
		Exit();
	}
	Printf("-------------------------------------------------------------------------------------\n\n");

	// Create beyond Me 
	Printf("-------------------------------------------------------------------------------------\n");
	Printf("makeprocs (%d): Creating process to access beyond memory range\n", getpid());
	process_create(PROCS1, s_procs_completed_str, NULL);
	if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
		Exit();
	}
	Printf("-------------------------------------------------------------------------------------\n\n");

	// Create Seg Fault
	Printf("-------------------------------------------------------------------------------------\n");
	Printf("makeprocs (%d): Creating process to access unallocated space\n", getpid());
	process_create(PROCS2, s_procs_completed_str, NULL);
	if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
		Exit();
	}
	Printf("-------------------------------------------------------------------------------------\n\n");


	// Create stack grow
	Printf("-------------------------------------------------------------------------------------\n");
	Printf("makeprocs (%d): Creating process to cause stack to grow\n", getpid());
	process_create(PROCS3, s_procs_completed_str, NULL);
	if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
		Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
		Exit();
	}
	Printf("-------------------------------------------------------------------------------------\n\n");


	// Create Hello World processes
	Printf("-------------------------------------------------------------------------------------\n");
	Printf("makeprocs (%d): Creating %d hello world's in a row, but only one runs at a time\n", getpid(), num_hello_world);
	for(i=0; i<num_hello_world; i++) {
		Printf("makeprocs (%d): Creating hello world #%d\n", getpid(), i);
		process_create(HELLO_WORLD, s_procs_completed_str, NULL);
		if (sem_wait(s_procs_completed) != SYNC_SUCCESS) {
			Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
			Exit();
		}
	}
	Printf("-------------------------------------------------------------------------------------\n");
	Printf("-------------------------------------------------------------------------------------\n\n");


	// Create Programs
	Printf("makeprocs (%d): Creating %d instances of Program. All run simultaneously.\n", getpid(), INST);
	for(i=0; i<INST; i++) {
		Printf("makeprocs (%d): Creating Program #%d\n", getpid(), i);
		process_create(PROG, s_prog_complete_str, NULL);
	}

	Printf("makeprocs (%d): I am done creating all Programs\n", getpid());
	Printf("makeprocs (%d): I will wait for all Programs to finish now.\n", getpid());
	Printf("-------------------------------------------------------------------------------------\n");
	for(i=0; i<INST; i++) {
		if (sem_wait(s_prog_complete) != SYNC_SUCCESS) {
			Printf("Bad semaphore s_prog_complete (%d) in %s\n", s_prog_complete, argv[0]);
			Exit();
		}
	}
	Printf("-------------------------------------------------------------------------------------\n");


	Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
