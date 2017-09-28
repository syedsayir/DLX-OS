#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);
int MemoryAllocPage(void);
 uint32 MemorySetupPte (uint32 page);

//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
void MemoryFreePage(uint32 page, PCB *pcb);
int MemoryROPAccessHandler(PCB *pcb);
void incCountRef (uint32 page);
void decCountRef (uint32 page);
void FreePPage(uint32 ppagenum );
uint32 getL2PTAddress();


//---------------------------------------------------------
// Put your here
//---------------------------------------------------------
typedef struct L2tables {
	uint32 inUse;
	uint32 pages[MEM_L2TABLE_SIZE];
} L2tables;
	void prFrPa();

#endif	// _memory_h_




