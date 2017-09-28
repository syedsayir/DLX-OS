#include "usertraps.h"
#include "misc.h"

#include "newfs.h"

dfs_inode inodes[DFS_INODE_MAX_NUM];
dfs_superblock sb;
uint32 fbv[DFS_FBV_MAX_NUM_WORDS];

int diskblocksize = 0; // These are global in order to speed things up
int disksize = 0;      // (i.e. fewer traps to OS to get the same number)

int FdiskWriteBlock(uint32 blocknum, dfs_block *b); //You can use your own function. This function 
//calls disk_write_block() to write physical blocks to disk

void main (int argc, char *argv[])
{
	// STUDENT: put your code here. Follow the guidelines below. They are just the main steps. 
	// You need to think of the finer details. You can use bzero() to zero out bytes in memory
	//Initializations and argc check
	int num_filesystem_blocks;
	int i;
	disksize = disk_size();
	diskblocksize = disk_blocksize();
	num_filesystem_blocks = disksize/DFS_BLOCKSIZE;

	disk_create();
	
	// Need to invalidate filesystem before writing to it to make sure that the OS
	// doesn't wipe out what we do here with the old version in memory
	// You can use dfs_invalidate(); but it will be implemented in Problem 2. You can just do 
	
	
	// Make sure the disk exists before doing anything else
	

	// Write all inodes as not in use and empty (all zeros)
	for(i=0;i<DFS_INODE_MAX_NUM;i++)
		inodes[i].inUse=0;
	for(i=0;i<FDISK_INODE_NUM_BLOCKS;i++)
		FdiskWriteBlock( FDISK_INODE_BLOCK_START+i , (dfs_block *)((char *)inodes + (i * DFS_BLOCKSIZE)) );


	// Next, setup free block vector (fbv) and write free block vector to the disk
	fbv[0]=0x0;
	fbv[1]=0xffffe000;
	for(i=2;i<DFS_FBV_MAX_NUM_WORDS;i++)
		fbv[i]=0xffffffff;
	//check this -- need to get rid of DFS_FBV_MAX_NUM_BLOCKS
	for(i=0;i<DFS_FBV_MAX_NUM_BLOCKS;i++)
		FdiskWriteBlock(FDISK_FBV_BLOCK_START+i , (dfs_block *)( ((char *)fbv) + i*DFS_BLOCKSIZE ));
	
	// Finally, setup superblock as valid filesystem and write superblock and boot record to disk: 
	// boot record is all zeros in the first physical block, and superblock structure goes into the second physical block
	sb.valid = 1; //check this
	sb.FSblocksize = DFS_BLOCKSIZE;
	sb.FSnumblocks = num_filesystem_blocks;
	sb.FSinode_beg = FDISK_INODE_BLOCK_START;
	sb.FSinodesnum = FDISK_NUM_INODES;
	sb.FSblkvec_beg = FDISK_FBV_BLOCK_START;
	disk_write_block(1, (char*) &sb);

	dfs_invalidate(); //check this


	Printf("fdisk (%d): Formatted DFS disk for %d bytes.\nDisk BlockSize: %d\n\n", getpid(), disksize,diskblocksize);
}

int FdiskWriteBlock(uint32 blocknum, dfs_block *b) {
	// STUDENT: put your code here
	disk_write_block(blocknum*2, b->data);
	disk_write_block(blocknum*2+1, b->data+diskblocksize); //check this
	return 0;
}
