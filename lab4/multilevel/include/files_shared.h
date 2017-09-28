#ifndef __FILES_SHARED__
#define __FILES_SHARED__

#define FILE_SEEK_SET 1
#define FILE_SEEK_END 2
#define FILE_SEEK_CUR 3


#define	FILE_MODE_READ	0x1
#define	FILE_MODE_WRITE	0x2
#define	FILE_MODE_RW	(FILE_MODE_READ | FILE_MODE_WRITE)


#define	FILE_EOF_REACHED	1
#define	FILE_EOF_NOT_REACHED	0


#define FILE_MAX_FILENAME_LENGTH 76

#define FILE_MAX_READWRITE_BYTES 4096

typedef struct file_descriptor {
  // STUDENT: put file descriptor info here
  char fileName[FILE_MAX_FILENAME_LENGTH];
  int iNode;
  int currPos;
  int eof;
  int mode;
  int pid;
  int inUse;
} file_descriptor;


#define FILE_FAIL -1
#define FILE_EOF -1
#define FILE_SUCCESS 1

#endif
