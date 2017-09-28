#ifndef __DFS_SHARED__
#define __DFS_SHARED__

#define DFS_BLOCKSIZE 512  // Must be an integer multiple of the disk blocksize
#define FNAME_MAX_LEN 72
#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB

#define DFS_FAIL -1
#define DFS_SUCCESS 1

//check this -- all are defined here?
#define SB_IDX 1 //location of Superblock in disk
#define DFS_INODE_MAX_NUM 192
#define DFS_FBV_MAX_NUM_WORDS (DFS_MAX_FILESYSTEM_SIZE/(DFS_BLOCKSIZE*32)) //(16MB/(512*32)) = 1024
#define DFS_FBV_MAX_NUM_BLOCKS ((DFS_FBV_MAX_NUM_WORDS*4)/DFS_BLOCKSIZE) //1024*4/512 = 8


//Inode for the root directory
#define ROOT_DIR 	0x0

#ifndef FILE
#define FILE		1
#endif
#ifndef DIR
#define DIR		2
#endif

#define TRUE	1
#define FALSE   0
#define R 1
#define W 2
#define RW 3


#define OR 4
#define OW 2
#define OX 1
#define UR 32
#define UW 16
#define UX 8
#define OK 1



typedef struct dfs_superblock {
  // STUDENT: put superblock internals here
  int valid;
  int FSblocksize;
  int FSnumblocks; //number of logical blocks in disk
  int FSinode_beg;
  int FSinodesnum;
  int FSblkvec_beg;
  char Allign[232]; //check this -- for any changes made to the superblock
} dfs_superblock;

typedef struct dfs_block {
  char data[DFS_BLOCKSIZE];
} dfs_block;

typedef struct dfs_inode {
  // STUDENT: put inode structure internals here
  // IMPORTANT: sizeof(dfs_inode) MUST return 96 in order to fit in enough
  // inodes in the filesystem (and to make your life easier).  To do this, 
  // adjust the maximumm length of the filename until the size of the overall inode 
  // is 96 bytes.
  unsigned char type; 		// directory or regular file
  unsigned char permission; 	// file permission
  unsigned char ownerId;
  unsigned char inUse;
  int size;
  unsigned int num_blocks;
  unsigned int dir_addrs[10];
  unsigned int indr_addrs;
  unsigned int indr_2_addrs;
  char pad[36];
} dfs_inode;

#endif
