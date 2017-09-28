#ifndef __DFS_SHARED__
#define __DFS_SHARED__

#define DFS_BLOCKSIZE 512  // Must be an integer multiple of the disk blocksize
#define FNAME_MAX_LEN 36
#define DFS_MAX_FILESYSTEM_SIZE 0x1000000  // 16MB

#define DFS_FAIL -1
#define DFS_SUCCESS 1

//check this -- all are defined here?
#define SB_IDX 1 //location of Superblock in disk
#define DFS_INODE_MAX_NUM 192
#define DFS_FBV_MAX_NUM_WORDS (DFS_MAX_FILESYSTEM_SIZE/(DFS_BLOCKSIZE*32)) //(16MB/(512*32)) = 1024
#define DFS_FBV_MAX_NUM_BLOCKS ((DFS_FBV_MAX_NUM_WORDS*4)/DFS_BLOCKSIZE) //1024*4/512 = 8

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
  int inUse;
  int size;
  unsigned int num_blocks;
  unsigned int dir_addrs[10];
  unsigned int indr_addrs;
  unsigned int indr_2_addrs;
  char filename[FNAME_MAX_LEN];
} dfs_inode;

#endif
