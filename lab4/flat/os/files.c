
#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "dfs.h"
#include "files.h"
#include "synch.h"

// You have already been told about the most likely places where you should use locks. You may use 
// additional locks if it is really necessary.
lock_t fileLock;

// STUDENT: put your file-level functions here


static file_descriptor fds[FILE_MAX_OPEN_FILES];

void FileModuleInit() {
	int i;
	fileLock = LockCreate();
	for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
		fds[i].inUse = -1;
		fds[i].pid = -1;
	}

}


int FileOpen(char* fileName, char* mode) {
	int mode_rw = FILE_FAIL;
	int i;
	int inode;
	int currPid;
	if (StrCmp(mode, "r")) mode_rw = FILE_MODE_READ;
	if (StrCmp(mode, "w")) mode_rw = FILE_MODE_WRITE;
	if (StrCmp(mode, "rw")) mode_rw = FILE_MODE_RW;
	if (mode_rw == FILE_FAIL) {
		printf("ERROR: Need to open file as r/w/rw not as %s.\n", mode);
		return FILE_FAIL;
	}

	LockHandleAcquire(fileLock);
	currPid = GetCurrentPid();
	// check this need to lock -- done
	inode = DfsInodeFilenameExists(fileName);
	if (inode != DFS_FAIL) {
		for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
			if (fds[i].inUse == 1 && fds[i].iNode == inode) {
				if (!(StrCmp(fileName, fds[i].fileName))) {
					printf("FATAL: Unknown error occured\n");
					LockHandleRelease(fileLock);
					return DFS_FAIL;
				}
				if (fds[i].pid == currPid) {
					printf("You already Opened this file..\n");
					LockHandleRelease(fileLock);
					return i;
				}
				else {
					printf("ERROR: FAILED. Pid: %d already has this open.\n", fds[i].pid);
					LockHandleRelease(fileLock);
					return FILE_FAIL; 
				}
			}
		}
		// File exists but not open
		if (mode_rw != FILE_MODE_READ) {
			DfsInodeDelete(inode);
			inode = DfsInodeOpen(fileName);
		}
		for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
			if (fds[i].inUse == -1) {
				dstrcpy(fds[i].fileName, fileName);
				fds[i].iNode = inode;
				fds[i].currPos = 0;
				fds[i].eof = FILE_EOF_NOT_REACHED;
				fds[i].mode = mode_rw;
				fds[i].pid = currPid;
				fds[i].inUse = 1;
				LockHandleRelease(fileLock);
				return i;
			}
		}
		printf("FATAL: Out of File Descriptor Structures.\n");
		LockHandleRelease(fileLock);
		return DFS_FAIL;
	}
	printf("Filename Doesn't Exist. Trying to create file.\n");
	inode = DfsInodeOpen(fileName);
	if (inode == DFS_FAIL) {
		printf("FATAL: Failed to create file.\n");
		LockHandleRelease(fileLock);
		return FILE_FAIL;
	}
	for (i = 0; i < FILE_MAX_OPEN_FILES; i++) {
		if (fds[i].inUse == -1) {
			dstrcpy(fds[i].fileName, fileName);
			fds[i].iNode = inode;
			fds[i].currPos = 0;
			fds[i].eof = FILE_EOF_NOT_REACHED;
			fds[i].mode = mode_rw;
			fds[i].pid = currPid;
			fds[i].inUse = 1;
			LockHandleRelease(fileLock);
			return i;
		}
	}
	printf("FATAL: Out of File Descriptor Structures.\n");
	LockHandleRelease(fileLock);
	return DFS_FAIL;
}

int FileClose(int handle) {
	if (handle < 0 || handle >= FILE_MAX_OPEN_FILES) {
		printf("ERROR: Trying to close invalid file descriptor.\n");
		return FILE_FAIL;
	}
	if (fds[handle].pid != GetCurrentPid()) {
		printf("ERROR: You dont have the file open!\n");
		return FILE_FAIL;
	}
	//check this need to lock -- no need
	fds[handle].inUse = -1;
	return FILE_SUCCESS;
}

int FileRead(int handle, void* mem, int num_bytes) {
	int size, currPos, bytesRead,i;
	if (handle < 0 || handle >= FILE_MAX_OPEN_FILES) {
		printf("ERROR: Invalid file descriptor passed.\n");
		return FILE_FAIL;
	}
	if (fds[handle].pid != GetCurrentPid()) {
		printf("ERROR: You dont have the file open!\n");
		return FILE_FAIL;
	}
	if (fds[handle].mode == FILE_MODE_WRITE) {
		printf("ERROR: You dont have read permission!\n");
		return FILE_FAIL;
	}
	if (num_bytes > FILE_MAX_READWRITE_BYTES) {
		printf("ERROR: Can't read more than 4Kbytes!\n");
		return FILE_FAIL;
	}

	size = DfsInodeFilesize(fds[handle].iNode);
		printf("ERROR: %d!\n",size);
		printf("ERROR: %d!\n",fds[handle].currPos );

	if (fds[handle].currPos == size) {
		fds[handle].eof = FILE_EOF_REACHED;
		printf("ERROR: Can't read. EndOfFile reached!\n");
		return FILE_FAIL;
	}

	currPos = fds[handle].currPos;
	bytesRead = DfsInodeReadBytes(fds[handle].iNode, mem, currPos, num_bytes);

	if (bytesRead == DFS_FAIL) {
		printf("Error: Couldn't read from Inode!\n");
		return FILE_FAIL;
	}
	currPos += bytesRead;
	if (currPos == size) {
		printf("Error: Reached End of the file.\n");
		fds[handle].eof = FILE_EOF_REACHED;
		fds[handle].currPos = size;
		//	for(i=0;i<1024;i++)
		//	printf("%c ,",((char *)mem)[i]);

	//	return FILE_FAIL;
	}
	return bytesRead;
}

int FileSeek(int handle, int num_bytes, int from_where) {
	int size, currPos;
	if (handle < 0 || handle >= FILE_MAX_OPEN_FILES) {
		printf("ERROR: Invalid file descriptor passed.\n");
		return FILE_FAIL;
	}
	if (fds[handle].pid != GetCurrentPid()) {
		printf("ERROR: You dont have the file open!\n");
		return FILE_FAIL;
	}
	if ((from_where != FILE_SEEK_SET) && (from_where != FILE_SEEK_END) && (from_where != FILE_SEEK_CUR)) {
		printf("ERROR: Incorrect from_where passed!\n");
		return FILE_FAIL;
	}
	size = DfsInodeFilesize(fds[handle].iNode);


	if (from_where == FILE_SEEK_END) {
		currPos = size - num_bytes;
	}
	else if (from_where == FILE_SEEK_SET) {
		currPos = num_bytes;
	}
	else {
		currPos = fds[handle].currPos + num_bytes;
	}

	if (currPos < 0) { 
		printf("ERROR: Attempting to seek beyond Beginning of File!\n");
		fds[handle].currPos = 0;
		return FILE_FAIL;
	}
	if (currPos >= size) {
		printf("ERROR: Attempting to seek beyond End of File!\n");
		fds[handle].currPos = size;
		return FILE_FAIL;
	}
	fds[handle].currPos = currPos;
	fds[handle].eof = FILE_EOF_NOT_REACHED;
	//printf("CurrPos:%d\n",currPos);
	return FILE_SUCCESS;
}


int FileWrite(int handle, void* mem, int num_bytes) {
	int currPos, bytesWritten;
	if (handle < 0 || handle >= FILE_MAX_OPEN_FILES) {
		printf("ERROR: Invalid file descriptor passed.\n");
		return FILE_FAIL;
	}
	if (fds[handle].pid != GetCurrentPid()) {
		printf("ERROR: You dont have the file open!\n");
		return FILE_FAIL;
	}
	if (fds[handle].mode == FILE_MODE_READ) {
		printf("ERROR: You dont have write permission!\n");
		return FILE_FAIL;
	}
	if (num_bytes > FILE_MAX_READWRITE_BYTES) {
		printf("ERROR: Can't write more than 4Kbytes!\n");
		return FILE_FAIL;
	}

	currPos = fds[handle].currPos;
	bytesWritten = DfsInodeWriteBytes(fds[handle].iNode, mem, currPos, num_bytes);

	if (bytesWritten == DFS_FAIL) {
		printf("Error: Couldn't write to Inode!\n");
		return FILE_FAIL;
	}
	currPos += bytesWritten;
	return bytesWritten;
}


int FileDelete(char* fileName){
	//check this -- need to lock before the delete is done
	if (DfsInodeFilenameExists(fileName) == DFS_FAIL) {
		printf("Error: Filename doesn't exist!\n");
		return FILE_FAIL;
	}
	printf("Deleting file...\n");
	DfsInodeDelete(DfsInodeOpen(fileName));
	return FILE_SUCCESS;
}

