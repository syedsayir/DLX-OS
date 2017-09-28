#ifndef __FILES_H__
#define __FILES_H__

#include "dfs.h"
#include "files_shared.h"
#include "filesys.h" //check this

#define FILE_MAX_OPEN_FILES 15


void FileModuleInit();
int FileOpen(char* fileName, char* mode);
int FileClose(int handle);
int FileRead(int handle, void* mem, int num_bytes);
int FileSeek(int handle, int num_bytes, int from_where);
int FileWrite(int handle, void* mem, int num_bytes);
int FileDelete(char* fileName);

#endif
