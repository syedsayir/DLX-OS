#ifndef __usertraps_h__
#define __usertraps_h__

//---------------------------------------------------------------------
// Function declarations for user traps defined in the assembly 
// library are provided here.  User-level programs can use any of 
// these functions, and any new traps need to have their declarations
// listed here.
//---------------------------------------------------------------------

int Open(char *filename, int arg2);
void Printf(char *format, ...);
void Exit();

// added for lab2
unsigned int Getpid();

#endif
