#include "ostraps.h"
#include "dlxos.h"
#include "traps.h"
#include "queue.h"
#include "disk.h"
#include "dfs.h"
#include "synch.h"
#include "misc.h"

static dfs_inode inodes[DFS_INODE_MAX_NUM/*specify size*/ ]; // all inodes
static dfs_superblock sb; // superblock
static uint32 fbv[DFS_FBV_MAX_NUM_WORDS/*specify size*/]; // Free block vector

static uint32 dfs_fbv_num_words;

static uint32 negativeone = 0xFFFFFFFF;
static inline uint32 invert(uint32 n) { return n ^ negativeone; }



lock_t fbvLock;
lock_t inodeLock;

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.

// STUDENT: put your file system level functions below.
// Some skeletons are provided. You can implement additional functions.

///////////////////////////////////////////////////////////////////
// Non-inode functions first
///////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------

//-----------------------------------------------------------------
// DfsInavlidate marks the current version of the filesystem in
// memory as invalid.  This is really only useful when formatting
// the disk, to prevent the current memory version from overwriting
// what you already have on the disk when the OS exits.
//-----------------------------------------------------------------

void DfsInvalidate() {
	// This is just a one-line function which sets the valid bit of the 
	// superblock to 0.
	sb.valid = 0;

}

//-------------------------------------------------------------------
// DfsOpenFileSystem loads the file system metadata from the disk
// into memory.  Returns DFS_SUCCESS on success, and DFS_FAIL on 
// failure.
//-------------------------------------------------------------------

int DfsOpenFileSystem() {
	int i;
	int bound; 
	int ratio;
	//Basic steps:
	// Check that filesystem is not already open
	// check this --- disk sb should be invalid and mem should be valid
	if (sb.valid)
		return DFS_FAIL;

	// Read superblock from disk.  Note this is using the disk read rather 
	// than the DFS read function because the DFS read requires a valid 
	// filesystem in memory already, and the filesystem cannot be valid 
	// until we read the superblock. Also, we don't know the block size 
	// until we read the superblock, either.

	// Copy the data from the block we just read into the superblock in memory
	DiskReadBlock(SB_IDX, (disk_block *)(&sb));
	ratio = sb.FSblocksize / DiskBytesPerBlock();

	// All other blocks are sized by virtual block size:
	// Read inodes
	bound = (sb.FSinodesnum * sizeof(dfs_inode)) / DiskBytesPerBlock();
	for (i=0; i<bound; i++) {
		DiskReadBlock(sb.FSinode_beg * ratio + i,(disk_block *)((char *)inodes + i * DiskBytesPerBlock()));	
	}
	// Read free block vector
	bound = (sb.FSnumblocks * 4)/(32*DiskBytesPerBlock());
	//check this bound for constants
	for (i=0; i<bound; i++) {
		DiskReadBlock(sb.FSblkvec_beg * ratio + i, (disk_block *) ((char *) fbv + i * DiskBytesPerBlock()));	
	}
	// Change superblock to be invalid, write back to disk, then change 
	// it back to be valid in memory

	sb.valid=0;
	DiskWriteBlock(SB_IDX,(disk_block *)(&sb));
	sb.valid=1;

	dfs_fbv_num_words = ceil(sb.FSnumblocks/32.0);
	return DFS_SUCCESS;

}
//----------------------------------------------------------------
// DfsModuleInit is called at boot time to initialize things and
// open the file system for use.
//-----------------------------------------------------------------

void DfsModuleInit() {
	// You essentially set the file system as invalid and then open 
	// using DfsOpenFileSystem().
	sb.valid=0;
	DfsOpenFileSystem();
	fbvLock = LockCreate();
	inodeLock = LockCreate();

}



//-------------------------------------------------------------------
// DfsCloseFileSystem writes the current memory version of the
// filesystem metadata to the disk, and invalidates the memory's 
// version.
//-------------------------------------------------------------------

int DfsCloseFileSystem() {

	int i;
	int bound =  (sb.FSinodesnum*sizeof(dfs_inode))/DiskBytesPerBlock();
	int ratio = sb.FSblocksize / DiskBytesPerBlock();
	printf("sb.valid is%d\n",sb.valid);
	if(!sb.valid) return DFS_FAIL;
	for (i=0; i<bound; i++) {
		DiskWriteBlock	(sb.FSinode_beg * ratio + i, (disk_block *) ((char *)inodes + i * DiskBytesPerBlock()));	
	}
	// Read free block vector
	bound=(sb.FSnumblocks*4)/(32*DiskBytesPerBlock());
	for (i=0; i<bound; i++) {
		DiskWriteBlock(sb.FSblkvec_beg * ratio + i, (disk_block *) ((char *) fbv + i * DiskBytesPerBlock()));	
	}
	// Change superblock to be invalid, write back to disk, then change 
	// it back to be valid in memory

	DiskWriteBlock(SB_IDX,(disk_block *)(&sb));
	sb.valid=0;

	return DFS_SUCCESS;

}


//-----------------------------------------------------------------
// DfsAllocateBlock allocates a DFS block for use. Remember to use 
// locks where necessary.
//-----------------------------------------------------------------

int DfsAllocateBlock() {
	// Check that file system has been validly loaded into memory
	// Find the first free block using the free block vector (FBV), mark it in use
	// Return handle to block
	// check this --- for locks -- done
	int i, j, ret;
	if(!sb.valid) return DFS_FAIL;
	LockHandleAcquire(fbvLock);
	for(i=0;i<dfs_fbv_num_words;i++) {
		if(fbv[i]) {
			for(j=0;j<32;j++){
				if(fbv[i]&(1<<j)){
					fbv[i]&=invert(1<<j);
					ret = i*32+j;
					if (ret >= sb.FSnumblocks) {
						LockHandleRelease(fbvLock);
						return DFS_FAIL;
					}
					LockHandleRelease(fbvLock);
					return ret;
				}
			}
		}
	}	
	LockHandleRelease(fbvLock);
	return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsFreeBlock deallocates a DFS block.
//-----------------------------------------------------------------

int DfsFreeBlock(uint32 blocknum) {

	if(fbv[blocknum/32]&(1<<(blocknum%32))) return DFS_FAIL;
	if(blocknum >= sb.FSnumblocks) return DFS_FAIL;

	if(!sb.valid) return DFS_FAIL;

	LockHandleAcquire(fbvLock);
	fbv[blocknum/32]|=1<<(blocknum%32);
	LockHandleRelease(fbvLock);
	return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsReadBlock reads an allocated DFS block from the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to read from it.  Returns DFS_FAIL
// on failure, and the number of bytes read on success.  
//-----------------------------------------------------------------

int DfsReadBlock(uint32 blocknum, dfs_block *b) {
	int i;
	int ratio = sb.FSblocksize / DiskBytesPerBlock();

	if (!sb.valid) return DFS_FAIL;
	if(blocknum >= sb.FSnumblocks) return DFS_FAIL;
	if(fbv[blocknum/32] & (1<<(blocknum%32))) return DFS_FAIL;

	for(i=0;i<sb.FSblocksize/DiskBytesPerBlock();i++)
		DiskReadBlock(blocknum * ratio + i, (disk_block *) ((char*) b + i * DiskBytesPerBlock()));
	return sb.FSblocksize;


}


//-----------------------------------------------------------------
// DfsWriteBlock writes to an allocated DFS block on the disk
// (which could span multiple physical disk blocks).  The block
// must be allocated in order to write to it.  Returns DFS_FAIL
// on failure, and the number of bytes written on success.  
//-----------------------------------------------------------------

int DfsWriteBlock(uint32 blocknum, dfs_block *b){
	int i;
	int ratio = sb.FSblocksize / DiskBytesPerBlock();

	if (!sb.valid) return DFS_FAIL;
	if(blocknum >= sb.FSnumblocks) {
		printf("%d %d\n\n", blocknum, sb.FSnumblocks);
		return DFS_FAIL;
	}
	if(fbv[blocknum/32] & (1<<(blocknum%32))) return DFS_FAIL;

	for(i=0;i<sb.FSblocksize/DiskBytesPerBlock();i++)
		DiskWriteBlock(blocknum * ratio + i, (disk_block *)((char*) b + i * DiskBytesPerBlock()));
	return sb.FSblocksize;
}


////////////////////////////////////////////////////////////////////////////////
// Inode-based functions
////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------
// DfsInodeFilenameExists looks through all the inuse inodes for 
// the given filename. If the filename is found, return the handle 
// of the inode. If it is not found, return DFS_FAIL.
//-----------------------------------------------------------------

int DfsInodeFilenameExists(char *filename) {
	int i;
	if (!sb.valid) return DFS_FAIL;
	for(i=0;i<sb.FSinodesnum;i++) {
		if(inodes[i].inUse) {
			if(StrCmp(filename,inodes[i].filename))
				return i;
		}
	}
	return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsInodeOpen: search the list of all inuse inodes for the 
// specified filename. If the filename exists, return the handle 
// of the inode. If it does not, allocate a new inode for this 
// filename and return its handle. Return DFS_FAIL on failure. 
// Remember to use locks whenever you allocate a new inode.
//-----------------------------------------------------------------

int DfsInodeOpen(char *filename) {
	int i,ret;
	if (!sb.valid) return DFS_FAIL;
	if((ret=DfsInodeFilenameExists(filename))!=DFS_FAIL)
		return ret;

	if (dstrlen(filename) >= FNAME_MAX_LEN) {
		printf("Can't have more than %d chars for Filename.\n", (FNAME_MAX_LEN-1));
		return DFS_FAIL;
	}
		
	for(i=0;i<sb.FSinodesnum;i++)
		if(!inodes[i].inUse) {	
			LockHandleAcquire(inodeLock);
			inodes[i].inUse=1;
			inodes[i].size=0;
			inodes[i].num_blocks=0;
			dstrncpy(inodes[i].filename,filename,FNAME_MAX_LEN);	
			LockHandleRelease(inodeLock);
			return i;
		}
	return DFS_FAIL;
}


//-----------------------------------------------------------------
// DfsInodeDelete de-allocates any data blocks used by this inode, 
// including the indirect addressing block if necessary, then mark 
// the inode as no longer in use. Use locks when modifying the 
// "inuse" flag in an inode.Return DFS_FAIL on failure, and 
// DFS_SUCCESS on success.
//-----------------------------------------------------------------

int DfsInodeDelete(uint32 handle) {

	//check this.... use last function for this preferably might have bugs
	int numBlocks, i, j;
	dfs_block buf, buf2;
	int* p;
	int* p2;
	int intsperblock = sb.FSblocksize/sizeof(int);
	if (!sb.valid) return DFS_FAIL;
	if (handle >= DFS_INODE_MAX_NUM) return DFS_FAIL;
	if (!(inodes[handle].inUse)) return DFS_FAIL;
	numBlocks = ceil( inodes[handle].size/(float)(sb.FSblocksize));


	LockHandleAcquire(inodeLock);
	for(i=0; i<numBlocks; i++) {
		if (i == 10) break;
		DfsFreeBlock(inodes[handle].dir_addrs[i]);
	}
	numBlocks -= 10;
	if (numBlocks > 0){
		DfsReadBlock(inodes[handle].indr_addrs, &buf);
		p = (int *) &buf;
		for (i=0; i<numBlocks; i++) {
			if (i == intsperblock) break;
			DfsFreeBlock(p[i]);
		}
	}
	numBlocks -= intsperblock;
	if (numBlocks > 0){
		DfsReadBlock(inodes[handle].indr_2_addrs, &buf);
		p = (int *) &buf;
		for(i=0; i<intsperblock; i++) {
			DfsReadBlock(p[i], &buf2);
			p2 = (int *) &buf2;
			for(j=0; j<intsperblock; j++) {
				DfsFreeBlock(p2[j]);
				numBlocks--;
				if (numBlocks == 0) break;
			}
			if (numBlocks == 0) break;
		}
	}
	if (numBlocks > 0) {
		LockHandleRelease(inodeLock);
		return DFS_FAIL;
	}

	//check this -- need to lock before modifying -- done
	inodes[handle].inUse = 0;
	LockHandleRelease(inodeLock);
	return DFS_SUCCESS;
}


//-----------------------------------------------------------------
// DfsInodeReadBytes reads num_bytes from the file represented by 
// the inode handle, starting at virtual byte start_byte, copying 
// the data to the address pointed to by mem. Return DFS_FAIL on 
// failure, and the number of bytes read on success.
//-----------------------------------------------------------------

int DfsInodeReadBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {

	int startblk, endblk, bytes_to_read, blk;
	dfs_block buf;
	int ptr_mem, ptr_beg, ptr_end;
	int i;

	if (!sb.valid) return DFS_FAIL;
	if (handle >= DFS_INODE_MAX_NUM) return DFS_FAIL;
	if(!(inodes[handle].inUse)) return DFS_FAIL;


	startblk = start_byte/sb.FSblocksize;
	endblk = (start_byte+num_bytes <= inodes[handle].size) ? start_byte+num_bytes-1 : inodes[handle].size-1;
	endblk /= sb.FSblocksize;

	bytes_to_read=(start_byte+num_bytes < inodes[handle].size) ? num_bytes : (inodes[handle].size - start_byte ); // this for checking end of file

	ptr_beg = start_byte % sb.FSblocksize;
	ptr_end = (start_byte + bytes_to_read - 1 ) % sb.FSblocksize;
	ptr_mem = 0;

	for (i=startblk; i<=endblk; i++) {
		//printf("beg: %d, end: %d, mem: %d, bytes: %d ,str: %d,end: %d\n\n",ptr_beg,ptr_end, ptr_mem,bytes_to_read,startblk,endblk);
		blk = DfsInodeTranslateVirtualToFilesys(handle, i);
		DfsReadBlock(blk, &buf);

		if (i == startblk && i == endblk) {
			// most common case
			bcopy( ((char *) &buf + ptr_beg), mem, bytes_to_read);	
			ptr_mem +=  bytes_to_read;
		}
		else if (i == startblk) {
			bcopy( ((char *) &buf + ptr_beg), mem, (sb.FSblocksize - ptr_beg) );	
			ptr_mem += (sb.FSblocksize - ptr_beg);
		}
		else if (i == endblk) {
			bcopy( (char *)&buf, ((char *) mem) + ptr_mem, (ptr_end + 1) );	
			ptr_mem +=( ptr_end + 1 );
		}
		else {
			bcopy( (char *)&buf, ((char *) mem) + ptr_mem, sb.FSblocksize);	
			ptr_mem += (sb.FSblocksize);
		}
		//printf("The buffer read so far is: %x\n", ((int*)mem)[2]);
	}

	//printf("%d and %d should be same... \n\n", ptr_mem, bytes_to_read);
	return bytes_to_read;

}


//-----------------------------------------------------------------
// DfsInodeWriteBytes writes num_bytes from the memory pointed to 
// by mem to the file represented by the inode handle, starting at 
// virtual byte start_byte. Note that if you are only writing part 
// of a given file system block, you'll need to read that block 
// from the disk first. Return DFS_FAIL on failure and the number 
// of bytes written on success.
//-----------------------------------------------------------------

int DfsInodeWriteBytes(uint32 handle, void *mem, int start_byte, int num_bytes) {
	//increment size of file here
	//numblocks is increased in allocate block function not here
	int startblk, endblk, blk;
	dfs_block buf;
	int ptr_mem, ptr_beg, ptr_end, sizetrk;
	int i;
	int flag;

	if (!sb.valid) return DFS_FAIL;
	if (handle >= DFS_INODE_MAX_NUM) return DFS_FAIL;
	if(!(inodes[handle].inUse)) return DFS_FAIL;
	if(start_byte > (inodes[handle].size)) {
		printf("DFSInodeWriteBytes: cant start writing from beyond the end of file.\n");
		return DFS_FAIL;
	}


	startblk = start_byte/sb.FSblocksize;
	endblk = (start_byte+num_bytes-1) / sb.FSblocksize;


	//bytes_to_write=(start_byte+num_bytes < inodes[handle].size) ? num_bytes : (inodes[handle].size - start_byte ); // this for checking end of file

	ptr_beg = start_byte % sb.FSblocksize;
	ptr_end = (start_byte + num_bytes - 1 ) % sb.FSblocksize;
	ptr_mem = 0;
	sizetrk = inodes[handle].size - (start_byte + num_bytes);

	for (i=startblk; i<=endblk; i++) {
		flag = DONTREAD;
		//printf("beg: %d, end: %d, mem: %d, bytes: %d ,str: %d,end: %d\n\n",ptr_beg,ptr_end, ptr_mem,num_bytes,startblk,endblk);
		if ( (blk = DfsInodeTranslateVirtualToFilesys(handle, i)) == DFS_FAIL ) {
			if ( (blk = DfsInodeAllocateVirtualBlock(handle, i)) == DFS_FAIL ) {
				printf("FATAL: Disk full. Out of memory.\n");
				if ( ptr_mem + start_byte > DfsInodeFilesize(handle)) {
					inodes[handle].size = ptr_mem + start_byte;
				}
				return ptr_mem;
			}
		}
		//printf("block is: %d\n",blk);

		if ( (i == startblk && ptr_beg != 0) || (i == endblk && ptr_end != (sb.FSblocksize-1)) ) {
			flag = READ;
		}

		if (flag == READ) {
			//read the block so u dont corrupt it
			DfsReadBlock(blk, &buf);
		}

		if (i == startblk && i == endblk) {
			// most common case
			bcopy( mem, ((char *) &buf + ptr_beg), num_bytes);	
			ptr_mem +=  num_bytes ;
		}
		else if (i == startblk) {
			bcopy( mem, ((char *) &buf + ptr_beg), (sb.FSblocksize - ptr_beg) );	
			ptr_mem += (sb.FSblocksize - ptr_beg);
		}
		else if (i == endblk) {
			bcopy( ((char *) mem) + ptr_mem, (char *)&buf, (ptr_end + 1) );	
			ptr_mem +=( ptr_end + 1 );
		}
		else {
			bcopy( ((char *) mem) + ptr_mem, (char *)&buf, sb.FSblocksize);	
			ptr_mem += (sb.FSblocksize);
		}
		//printf("The buffer to be written so far is: %x\n", ((int*)mem)[2]);
		if (DfsWriteBlock(blk, &buf) == DFS_FAIL) {
			printf("Unable to write block.\n");
			if ( ptr_mem + start_byte > DfsInodeFilesize(handle)) {
				inodes[handle].size = ptr_mem + start_byte;
			}
			return ptr_mem;
		}

	}

	//printf("%d and %d should be same... \n\n", ptr_mem, num_bytes);
	if ( ptr_mem + start_byte > DfsInodeFilesize(handle)) {
		inodes[handle].size = ptr_mem + start_byte;
	}
	return num_bytes ;

}


//-----------------------------------------------------------------
// DfsInodeFilesize simply returns the size of an inode's file. 
// This is defined as the maximum virtual byte number that has 
// been written to the inode thus far. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeFilesize(uint32 handle) {
	if (!sb.valid) return DFS_FAIL;
	if (handle >= DFS_INODE_MAX_NUM) return DFS_FAIL;
	if(!(inodes[handle].inUse)) return DFS_FAIL;
	return inodes[handle].size;

}


//-----------------------------------------------------------------
// DfsInodeAllocateVirtualBlock allocates a new filesystem block 
// for the given inode, storing its blocknumber at index 
// virtual_blocknumber in the translation table. If the 
// virtual_blocknumber resides in the indirect address space, and 
// there is not an allocated indirect addressing table, allocate it. 
// Return DFS_FAIL on failure, and the newly allocated file system 
// block number on success.
//-----------------------------------------------------------------

int DfsInodeAllocateVirtualBlock(uint32 handle, uint32 virtual_blocknum) {

	int intsperblock = sb.FSblocksize/sizeof(int);
	int level1, level2;
	int level1_blk;
	dfs_block buf;
	int* p;
	int FSblock, FSindblock;

	if (!sb.valid) { return DFS_FAIL; }
	if (handle >= DFS_INODE_MAX_NUM) { return DFS_FAIL; }
	if(!(inodes[handle].inUse)) { return DFS_FAIL;}
	if (virtual_blocknum != inodes[handle].num_blocks){  return DFS_FAIL; } //allocating block = numblocks

	if( (FSblock = DfsAllocateBlock()) == DFS_FAIL) {
		printf("Out of Blocks.\n");
		return DFS_FAIL;
	}


	if ( virtual_blocknum < 10 ) {
		inodes[handle].dir_addrs[virtual_blocknum] = FSblock;
		inodes[handle].num_blocks++;
		return FSblock;
	}
	if ( virtual_blocknum >= 10 && virtual_blocknum < intsperblock+10 ) {
		if (virtual_blocknum == 10) {
			if( (FSindblock = DfsAllocateBlock()) == DFS_FAIL) {
				printf("Out of Blocks.\n");
				return DFS_FAIL;
			}
			inodes[handle].indr_addrs = FSindblock;
		}

		FSindblock = inodes[handle].indr_addrs;
		DfsReadBlock(FSindblock, &buf);
		p = (int *) &buf;
		p[virtual_blocknum - 10] = FSblock;
		if (DfsWriteBlock(FSindblock, &buf) == DFS_FAIL) {
			printf("Unable to write block.\n");
			return DFS_FAIL;
		}
	//printf("----virtual_blocknum%d FSBlock%d\n",virtual_blocknum, FSblock);
		inodes[handle].num_blocks++;
		return FSblock;
	}
	virtual_blocknum -= (intsperblock+10);
	if (virtual_blocknum == 0) {
		//allocating first level of double indirect conditionally
		if( (FSindblock = DfsAllocateBlock()) == DFS_FAIL) {
			printf("Out of Blocks.\n");
			return DFS_FAIL;
		}
		inodes[handle].indr_2_addrs = FSindblock;
	}
	level1 = virtual_blocknum / intsperblock;
	level2 = virtual_blocknum % intsperblock;
	DfsReadBlock(inodes[handle].indr_2_addrs, &buf);
	p = (int *) &buf;
	level1_blk=p[level1];
	if (level2 == 0) {
		//allocating second level of double indirect
		if( (FSindblock = DfsAllocateBlock()) == DFS_FAIL) {
			printf("Out of Blocks.\n");
			return DFS_FAIL;
		}
		p[level1] = FSindblock;
		if (DfsWriteBlock(inodes[handle].indr_2_addrs, &buf) == DFS_FAIL) {
			printf("Unable to write block.\n");
			return DFS_FAIL;
		}
		level1_blk=p[level1];
	}
	DfsReadBlock(level1_blk, &buf);
	p = (int *) &buf;
	//this is the real data block allocation -- very hard -- garbage
	p[level2] = FSblock;
	if (DfsWriteBlock(level1_blk, &buf) == DFS_FAIL) {
		printf("Unable to write block.\n");
		return DFS_FAIL;
	}
	inodes[handle].num_blocks++;
	return FSblock;
}



//-----------------------------------------------------------------
// DfsInodeTranslateVirtualToFilesys translates the 
// virtual_blocknum to the corresponding file system block using 
// the inode identified by handle. Return DFS_FAIL on failure.
//-----------------------------------------------------------------

int DfsInodeTranslateVirtualToFilesys(uint32 handle, uint32 virtual_blocknum) {

	int intsperblock = sb.FSblocksize/sizeof(int);
	int* p;
	dfs_block buf;
	if (virtual_blocknum >= inodes[handle].num_blocks) return DFS_FAIL;
	if (!sb.valid) return DFS_FAIL;
	if (handle >= DFS_INODE_MAX_NUM) return DFS_FAIL;


	if ( virtual_blocknum < 10 ) {
		return inodes[handle].dir_addrs[virtual_blocknum];
	}
	if ( virtual_blocknum >= 10 && virtual_blocknum < intsperblock+10 ) {
		if (DfsReadBlock(inodes[handle].indr_addrs, &buf) == DFS_FAIL) {
			printf ("TranslateVirtualToFilesys: Can't read block.\n");
			return DFS_FAIL;
		}
		p = (int *) &buf;
		return p[virtual_blocknum - 10];
	}
	virtual_blocknum -= (intsperblock+10);
	DfsReadBlock(inodes[handle].indr_2_addrs, &buf);
	p = (int *) &buf;
	DfsReadBlock(p[virtual_blocknum/intsperblock], &buf);
	p = (int *) &buf;
	return p[virtual_blocknum%intsperblock];

}







