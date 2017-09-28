//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
static uint32 freemap[MEM_MAX_PAGES/32];
//static uint32 pagestart;
static int nfreepages;
static int freemapmax;

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
  }

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
	return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryInitModule
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() {
	int i;
	int page_num;
	for (i=0; i<(MEM_MAX_PAGES/32); i++) {
		freemap[i] = 0;
	}
	page_num = (lastosaddress >> MEM_L1FIELD_FIRST_BITNUM);
	for (i=0; i<page_num; i++) {
		freemap[i/32] |= (1<<(i%32)); //one by one each bit of the freemap is set to 1
	}
	freemapmax = (MemoryGetSize() >> MEM_L1FIELD_FIRST_BITNUM);
	nfreepages = freemapmax - page_num;
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) {
	uint32 pageNum = addr>>MEM_L1FIELD_FIRST_BITNUM;
	if ((pageNum < MEM_L1TABLE_SIZE) && ((pcb->pagetable[pageNum]) & MEM_PTE_VALID))
		return ((pcb->pagetable[pageNum])&(invert(MEM_ADDRESS_OFFSET_MASK)))|(MEM_ADDRESS_OFFSET_MASK&addr);
	return 0;
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
	unsigned char *curUser;         // Holds current physical address representing user-space virtual address
	int		bytesCopied = 0;  // Running counter
	int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

	while (n > 0) {
		// Translate current user page to system address.  If this fails, return
		// the number of bytes copied so far.
		curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

		// If we could not translate address, exit now
		if (curUser == (unsigned char *)0) break;

		// Calculate the number of bytes to copy this time.  If we have more bytes
		// to copy than there are left in the current page, we'll have to just copy to the
		// end of the page and then go through the loop again with the next page.
		// In other words, "bytesToCopy" is the minimum of the bytes left on this page 
		// and the total number of bytes left to copy ("n").

		// First, compute number of bytes left in this page.  This is just
		// the total size of a page minus the current offset part of the physical
		// address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
		// MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
		// "offset" portion of an address.
		bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);

		// Now find minimum of bytes in this page vs. total bytes left to copy
		if (bytesToCopy > n) {
			bytesToCopy = n;
		}

		// Perform the copy.
		if (dir >= 0) {
			bcopy (system, curUser, bytesToCopy);
		} else {
			bcopy (curUser, system, bytesToCopy);
		}

		// Keep track of bytes copied and adjust addresses appropriately.
		n -= bytesToCopy;           // Total number of bytes left to copy
		bytesCopied += bytesToCopy; // Total number of bytes copied thus far
		system += bytesToCopy;      // Current address in system space to copy next bytes from/into
		user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
	}
	return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
	return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
	return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) {
	uint32 addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];

	// segfault if the faulting address is not part of the stack
	uint32 vpagenum = addr>>MEM_L1FIELD_FIRST_BITNUM;
	int ppagenum;
	uint32 stackpagenum = (pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER])>>MEM_L1FIELD_FIRST_BITNUM;
	if (vpagenum < stackpagenum) {
		dbprintf('m', "addr = %x\nsp = %x\n", addr, pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER]);
		//printf("FATAL ERROR (%d): segmentation fault at page address %x\n", findpid(pcb), addr);
		printf("FATAL ERROR (%d): segmentation fault at page address %x\n", GetPidFromAddress(pcb), addr);
		ProcessKill();
		return MEM_FAIL;
	}

	ppagenum = MemoryAllocPage();
	if (ppagenum == MEM_FAIL) {
		//check this........ process needs to be killed....
			ProcessKill();
			return MEM_FAIL;
	}
	printf("MEMORY: Allocated a page for the stack growth.\n");
	pcb->pagetable[vpagenum] = MemorySetupPte(ppagenum);
	dbprintf('m', "Returning from page fault handler\n");
	return MEM_SUCCESS;
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

int MemoryAllocPage(void) {
	int intrs;               // Stores previous interrupt settings.
	int i, j;
	int page_num = -1;
	intrs = DisableIntrs ();
	dbprintf ('I', "Old interrupt value was 0x%x.\n", intrs);
	if (nfreepages == 0) {
		RestoreIntrs (intrs);
		dbprintf('m', "OUT OF PHYSICAL MEM in MemoryAllocPage");
		return -1;
	}
	for (i=0; i<(freemapmax/32); i++) {
		if (freemap[i] != 0xFFFFFFFF) {
			for (j=0; j<32; j++) {
				if (((freemap[i])&(1<<j)) == 0 ) {
					freemap[i] |= (1<<j);
					page_num = i*32+j;
					nfreepages--;
					break;
				}
			}
			break;
		}
	}
	RestoreIntrs (intrs);
	dbprintf ('m', "Allocating Freeing physical page number: %d.\n", page_num);
	return page_num;
}


inline uint32 MemorySetupPte (uint32 page) {
	return (page<<MEM_L1FIELD_FIRST_BITNUM)|MEM_PTE_VALID;
}


void MemoryFreePage(uint32 page, PCB *pcb) {
	if ((pcb->pagetable[page]) & MEM_PTE_VALID) {
		int ppagenum = (pcb->pagetable[page]) >> MEM_L1FIELD_FIRST_BITNUM;
		pcb->pagetable[page] &= (invert(MEM_PTE_VALID));
		freemap[ppagenum/32] &= (invert(1<<(ppagenum%32)));
		nfreepages++;
		dbprintf ('m', "Freeing physical page number: %d.\n", ppagenum);
	}
}

void MemoryFreeStackPage(PCB *pcb) {
		int ppagenum = pcb->sysStackArea;
		freemap[ppagenum/32] &= (invert(1<<(ppagenum%32)));
		nfreepages++;
		dbprintf ('m', "Freeing physical page number: %d.\n", ppagenum);
}
