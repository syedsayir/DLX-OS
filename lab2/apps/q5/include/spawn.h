#ifndef __USERPROG__
#define __USERPROG__

typedef struct atmosphere {
	int n_atoms;
	int o_atoms;
	int n2_molecules;
	int o2_molecules;
	int no2_molecules;
	int o3_molecules;
} atmosphere;

#define FILENAME_TO_RUN_N "nprocess.dlx.obj"
#define FILENAME_TO_RUN_O "oprocess.dlx.obj"

#endif
